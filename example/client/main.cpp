#include <cmdline.h>

#include <iostream>
#include <thread>
#include <chrono>

/*-------------------------------------------------------------------------*/

#include "../serviceStructs.h"

//optionaly override the default milisecond timeout for the requests. Default is 30
//#define SERVMAK_CLI_DEFAULT_TIMEOUT 100 

#define SERVMAK_CLIENT_IMPLEMENTATION
#include <servmak_lib/servmak_client.h>

/*-------------------------------------------------------------------------*/

bool g_shouldQuit = false;

// Optional macros to declare the method in a header file
// SERVMAK_CALLBACK_DECLARE(myCallback);
// SERVMAK_CALLBACK_DECLARE(myUrgentCallback);

SERVMAK_CALLBACK_IMPLEMENT(myCallback, args,
{
    static uint8_t counter = 0;
    printf("MY CUSTOM CALLBACK %d\n", args->notification);
    counter++;

    if(counter == 5){
        g_shouldQuit = true;
    }
})

SERVMAK_CALLBACK_IMPLEMENT(myUrgentCallback, args,
{
    printf("MY URGENT CALLBACK. level %d. type %d", args->urgencyLevel, args->urgencyType);
})

/*-------------------------------------------------------------------------*/
int client(uint16_t serverPort, uint16_t clientPort){
    std::cout << "starting at port " << clientPort << std::endl;

    //------Start servmak client
    ServMak_cli_t ctx;
    int res = servmak_cli_init(&ctx, "127.0.0.1", serverPort, clientPort);
    if(res < 0){
        std::cout << "failed to init client." << servmak_cli_get_error() << std::endl;
        return 1;
    }

    //------>Register callBack methods
    servmak_register_callback(&ctx, myCallback);
    servmak_register_callback(&ctx, myUrgentCallback);


    //------Execute a remote method
    myMethod myArgs = {'X'};
    strcpy(myArgs.str, "hola");

    myMethod_ret_t reply;
    if(servmak_request(&ctx, myMethod, &myArgs, &reply))
        std::cout << "GOT DATA OK:" << reply <<  std::endl;
    else
        std::cout << "GOT DATA FAIL" << std::endl;

    //------Main Loop
    while(!g_shouldQuit){
        servmak_cli_loop(&ctx);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    }
    servmak_cli_shutdown(&ctx);

    return 0;
}

int main(int argc, char** argv){
    cmdline::parser cmdParser;
    cmdParser.add<int>("port", 'p', "port number",true);
    cmdParser.add<int>("client", 'c', "client port number", true, 0);

    cmdParser.parse_check(argc, argv);

    uint16_t serverPort = cmdParser.get<int>("port");
    uint16_t clientPort = cmdParser.get<int>("client");

    client(serverPort, clientPort);
  
}