#ifndef INCLUDE_SERVMAK_CLI_H
#define INCLUDE_SERVMAK_CLI_H

#ifndef SERVMAK_CLI_DEFAULT_TIMEOUT
#define SERVMAK_CLI_DEFAULT_TIMEOUT 30
#endif


#ifdef SERVMAK_CLI_STATIC
    #define SERVMAK_NET_STATIC
    #define SERVMAK_CLI_STATIC static
#else
    #define SERVMAK_CLI_STATIC extern
#endif

#include <inttypes.h>
#include <stdlib.h>


#define servmak_request(context, stName, argument, retPtr)\
    servmak_cli_req(context, SERVMAK_STRUCT_ID(stName), argument, SERVMAK_STRUCT_LEN(stName), retPtr, SERVMAK_STRUCT_RET_LEN(stName)) 

#define servmak_request_void(context, stName, argument)\
    servmak_cli_req(context, SERVMAK_STRUCT_ID(stName), argument, SERVMAK_STRUCT_LEN(stName), NULL , SERVMAK_STRUCT_RET_LEN(stName)) 

#define servmak_request_timeout(context, stName, argument, retPtr, msTimeout)\
    servmak_cli_req(context, SERVMAK_STRUCT_ID(stName), argument, SERVMAK_STRUCT_LEN(stName), retPtr, SERVMAK_STRUCT_RET_LEN(stName), msTimeout) 
        
#define servmak_register_callback(context, stName) servmak_cli_addCallback(context, SERVMAK_STRUCT_ID(stName), SERVMAK_HANDLER(stName))

typedef void (*servmak_ftor)(void* arg);
typedef struct _ServMak_cli_t_ ServMak_cli_t;

SERVMAK_CLI_STATIC const char *servmak_cli_get_error(void);
SERVMAK_CLI_STATIC int servmak_cli_init(ServMak_cli_t* ctx, const char *host, uint16_t serverPort, uint16_t clientPort);
SERVMAK_CLI_STATIC void servmak_cli_shutdown(ServMak_cli_t* ctx);
SERVMAK_CLI_STATIC bool servmak_cli_req(ServMak_cli_t* ctx, uint8_t methodId, void* data, uint8_t len, void* result = NULL, uint8_t resultLen = 0 ,uint16_t msTimeout = SERVMAK_CLI_DEFAULT_TIMEOUT);
SERVMAK_CLI_STATIC void servmak_cli_loop(ServMak_cli_t* ctx);
SERVMAK_CLI_STATIC bool servmak_cli_addCallback(ServMak_cli_t* ctx, uint8_t methodId, servmak_ftor ftor);

#endif

#if defined(SERVMAK_CLIENT_IMPLEMENTATION) && !defined(SERVMAK__OMIT_NET_IMPLEMETATION)
#define SERVMAK_NET_IMPLEMENTATION
#endif

#ifdef SERVMAK_CUSTOM_NET_LOCATION
#include SERVMAK_CUSTOM_NET_LOCATION
#else
#include "servmak_net.h"
#endif 

#ifndef INCLUDE_SERVMAK_CLI_H_OTHER
#define INCLUDE_SERVMAK_CLI_H_OTHER

#include <stdio.h>
#ifdef SERVMAK_CLI_CALLBACK_COUNT
    #define __SERVMAK_CLI_CALLBACK_COUNT SERVMAK_CLI_CALLBACK_COUNT
#else
    #define __SERVMAK_CLI_CALLBACK_COUNT 0
#endif

typedef struct _ServMak_cli_t_{
    servmak_net_socket_t socket;
    servmak_net_address_t address;
    servmak_ftor callbacks[__SERVMAK_CLI_CALLBACK_COUNT];
    uint8_t callbacks_ids[__SERVMAK_CLI_CALLBACK_COUNT] = {0xFE};
    void * obj_addr = NULL;
} ServMak_cli_t;

#endif

#ifdef SERVMAK_CLIENT_IMPLEMENTATION

#include <stdio.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

// Function to sleep cross-platform (in milliseconds)
void servmak_sleep_ms(int milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);  // Windows Sleep is in milliseconds
#else
    usleep(milliseconds * 1000);  // Unix-like systems use usleep (microseconds), so convert milliseconds to microseconds
#endif
}

// Function to get the current time in milliseconds
uint64_t servmak_current_time_ms() {
#ifdef _WIN32
    return GetTickCount64();  // Returns the system uptime in milliseconds
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000 + tv.tv_usec / 1000;  // Convert to milliseconds
#endif
}

    
static const char *servmak__g_error;

static int servmak_cli__error(const char *message) {
    servmak__g_error = message;

    return -1;
}

SERVMAK_NET_DEF const char *servmak_cli_get_error(void) {
    return servmak__g_error;
}

#include <unistd.h>

SERVMAK_NET_DEF int servmak_cli_init(ServMak_cli_t* ctx, const char *host, uint16_t serverPort, uint16_t clientPort){
    if(servmak_net_init() < 0)
        return servmak_cli__error(servmak_net_get_error());
    
    if(servmak_net_udp_socket_open(&ctx->socket, clientPort, 1) < 0){
        return servmak_cli__error(servmak_net_get_error());
    }
    if (servmak_net_get_address(&ctx->address, host, serverPort) != 0) {
        servmak_net_socket_close(&ctx->socket);
        servmak_net_shutdown();
        return servmak_cli__error(servmak_net_get_error());
    }

    uint8_t buffer[3] = {0xFE, 0xAA, 0x55};
    //servmak_net_udp_socket_send(&ctx->socket, ctx->address, buffer, sizeof(buffer));

    return 0;
}

SERVMAK_CLI_STATIC void servmak_cli_shutdown(ServMak_cli_t* ctx){
    servmak_net_socket_close(&ctx->socket);
    servmak_net_shutdown();
}

SERVMAK_CLI_STATIC bool servmak_cli_req(ServMak_cli_t* ctx, uint8_t methodId, void* data, uint8_t len, void* result, uint8_t resultLen ,uint16_t msTimeout){
    char buff[255];

    uint64_t endTime = servmak_current_time_ms() + msTimeout; 
    buff[0] = methodId;
    memcpy(buff+1, data, len);
    
    servmak_net_udp_socket_send(&ctx->socket, ctx->address, buff, len+1);

    if(resultLen == 0)
        return true; //TODO: add protocol wid verification

    servmak_net_address_t sender;
    int bytesRead = servmak_net_udp_socket_receive(&ctx->socket, &sender, buff, sizeof(buff));
    while(!bytesRead && servmak_current_time_ms() < endTime){
        bytesRead = servmak_net_udp_socket_receive(&ctx->socket, &sender, buff, sizeof(buff));
        servmak_sleep_ms(1);
    }

    if(!bytesRead && servmak_current_time_ms() >= endTime)
        return false;
    if(resultLen != bytesRead)
        return false;

    memcpy(result, buff, resultLen);
    return true;

}

SERVMAK_CLI_STATIC void servmak_cli_loop(ServMak_cli_t* ctx){
    servmak_net_address_t sender;
    char buff[255] = {0};
    int bytes_read = servmak_net_udp_socket_receive(&ctx->socket, &sender, buff, sizeof(buff));
    if(bytes_read < 1)
        return;

    //printf("Received %d bytes from %s:%d: %s\n", bytes_read, servmak_net_host_to_str(sender.host), sender.port, buff);
    for(uint8_t i = 0; i < __SERVMAK_CLI_CALLBACK_COUNT; i++){
        if(buff[0] != ctx->callbacks_ids[i])
            continue;
        
        ctx->callbacks[i]((void*)(buff+1));
        break;        
    }

}

SERVMAK_CLI_STATIC bool servmak_cli_addCallback(ServMak_cli_t* ctx, uint8_t methodId, servmak_ftor ftor){
    for(uint8_t i = 0; i < __SERVMAK_CLI_CALLBACK_COUNT; i++){
        if(ctx->callbacks_ids[i] != 0xFE)
            continue;
        
        ctx->callbacks_ids[i] = methodId;
        ctx->callbacks[i] = ftor;
        return true;
    }
    return false;
}

#endif