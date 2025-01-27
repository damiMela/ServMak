/////////////////////////////////////////////////////////////////////////////////////////
//
// servmak_net
//
// This code is based on the opensource library "zed_net"
//
// USAGE
//
//    #define the symbol SERVMAK_NET_IMPLEMENTATION in *one* C/C++ file before the #include
//    of this file; the implementation will be generated in that file.
//
//    If you define the symbol SERVMAK_NET_STATIC, then the implementation will be private to
//    that file.

#ifndef INCLUDE_SERVMAK_NET_H
#define INCLUDE_SERVMAK_NET_H


#ifdef SERVMAK_NET_STATIC
#define SERVMAK_NET_DEF static
#else
#define SERVMAK_NET_DEF extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////////////////
//
// INITIALIZATION AND SHUTDOWN
//

// Get a brief reason for failure
SERVMAK_NET_DEF const char *servmak_net_get_error(void);

// Perform platform-specific socket initialization;
// *must* be called before using any other function
//
// Returns 0 on success, -1 otherwise (call 'servmak_net_get_error' for more info)
SERVMAK_NET_DEF int servmak_net_init(void);

// Perform platform-specific socket de-initialization;
// *must* be called when finished using the other functions
SERVMAK_NET_DEF void servmak_net_shutdown(void);

/////////////////////////////////////////////////////////////////////////////////////////
//
// INTERNET ADDRESS API
//

// Represents an internet address usable by sockets
typedef struct {
    unsigned int host;
    unsigned short port;
} servmak_net_address_t;

// Obtain an address from a host name and a port
//
// 'host' may contain a decimal formatted IP (such as "127.0.0.1"), a human readable
// name (such as "localhost"), or NULL for the default address
//
// Returns 0 on success, -1 otherwise (call 'servmak_net_get_error' for more info)
SERVMAK_NET_DEF int servmak_net_get_address(servmak_net_address_t *address, const char *host, unsigned short port);

// Converts an address's host name into a decimal formatted string
//
// Returns NULL on failure (call 'servmak_net_get_error' for more info)
SERVMAK_NET_DEF const char *servmak_net_host_to_str(unsigned int host);

/////////////////////////////////////////////////////////////////////////////////////////
//
// SOCKET HANDLE API
//

// Wraps the system handle for a UDP/TCP socket
typedef struct {
    int handle;
    unsigned long non_blocking;
    int ready;
} servmak_net_socket_t;

// Closes a previously opened socket
SERVMAK_NET_DEF void servmak_net_socket_close(servmak_net_socket_t *socket);

/////////////////////////////////////////////////////////////////////////////////////////
//
// UDP SOCKETS API
//

// Opens a UDP socket and binds it to a specified port
// (use 0 to select a random open port)
//
// Socket will not block if 'non-blocking' is non-zero
//
// Returns 0 on success
// Returns -1 on failure (call 'servmak_net_get_error' for more info)
SERVMAK_NET_DEF int servmak_net_udp_socket_open(servmak_net_socket_t *socket, unsigned int port, unsigned long non_blocking);

// Sends a specific amount of data to 'destination'
//
// Returns 0 on success, -1 otherwise (call 'servmak_net_get_error' for more info)
SERVMAK_NET_DEF int servmak_net_udp_socket_send(servmak_net_socket_t *socket, servmak_net_address_t destination, const void *data, int size);

// Receives a specific amount of data from 'sender'
//
// Returns the number of bytes received, -1 otherwise (call 'servmak_net_get_error' for more info)
SERVMAK_NET_DEF int servmak_net_udp_socket_receive(servmak_net_socket_t *socket, servmak_net_address_t *sender, void *data, int size);

#ifdef __cplusplus
}
#endif
#endif // INCLUDE_SERVMAK_NET_H



#ifdef SERVMAK_NET_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment(lib, "wsock32.lib")
#define ZED_NET_SOCKET_ERROR SOCKET_ERROR
#define ZED_NET_INVALID_SOCKET INVALID_SOCKET
#else
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#define ZED_NET_SOCKET_ERROR -1
#define ZED_NET_INVALID_SOCKET -1
#endif

static const char *servmak_net__g_error;

static int servmak_net__error(const char *message) {
    servmak_net__g_error = message;

    return -1;
}

SERVMAK_NET_DEF const char *servmak_net_get_error(void) {
    return servmak_net__g_error;
}

SERVMAK_NET_DEF int servmak_net_init(void) {
#ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        return servmak_net__error("Windows Sockets failed to start");
    }

    return 0;
#else
    return 0;
#endif
}

SERVMAK_NET_DEF void servmak_net_shutdown(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}

SERVMAK_NET_DEF int servmak_net_get_address(servmak_net_address_t *address, const char *host, unsigned short port) {
    if (host == NULL) {
        address->host = INADDR_ANY;
    } else {
        address->host = inet_addr(host);
        if (address->host == INADDR_NONE) {
            struct hostent *hostent = gethostbyname(host);
            if (hostent) {
                memcpy(&address->host, hostent->h_addr, hostent->h_length);
            } else {
                return servmak_net__error("Invalid host name");
            }
        }
    }

    address->port = port;
    
    return 0;
}

SERVMAK_NET_DEF const char *servmak_net_host_to_str(unsigned int host) {
    struct in_addr in;
    in.s_addr = host;

    return inet_ntoa(in);
}

SERVMAK_NET_DEF int servmak_net_udp_socket_open(servmak_net_socket_t *sock, unsigned int port, unsigned long non_blocking) {
    if (!sock)
        return servmak_net__error("Socket is NULL");

    // Create the socket
    sock->handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock->handle <= 0) {
        servmak_net_socket_close(sock);
        return servmak_net__error("Failed to create socket");
    }

    // Bind the socket to the port
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(sock->handle, (const struct sockaddr *) &address, sizeof(struct sockaddr_in)) != 0) {
        servmak_net_socket_close(sock);
        return servmak_net__error("Failed to bind socket");
    }

    // Set the socket to non-blocking if neccessary
    if (non_blocking) {
#ifdef _WIN32
        if (ioctlsocket(sock->handle, FIONBIO, &non_blocking) != 0) {
            servmak_net_socket_close(sock);
            return servmak_net__error("Failed to set socket to non-blocking");
        }
#else
        if (fcntl(sock->handle, F_SETFL, O_NONBLOCK, non_blocking) != 0) {
            servmak_net_socket_close(sock);
            return servmak_net__error("Failed to set socket to non-blocking");
        }
#endif
    }

    sock->non_blocking = non_blocking;

    return 0;
}

SERVMAK_NET_DEF void servmak_net_socket_close(servmak_net_socket_t *socket) {
    if (!socket) {
        return;
    }

    if (socket->handle) {
#ifdef _WIN32
        closesocket(socket->handle);
#else
        close(socket->handle);
#endif
    }
}

SERVMAK_NET_DEF int servmak_net_udp_socket_send(servmak_net_socket_t *socket, servmak_net_address_t destination, const void *data, int size) {
    if (!socket) {
        return servmak_net__error("Socket is NULL");
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = destination.host;
    address.sin_port = htons(destination.port);

    int sent_bytes = sendto(socket->handle, (const char *) data, size, 0, (const struct sockaddr *) &address, sizeof(struct sockaddr_in));
    if (sent_bytes != size) {
        return servmak_net__error("Failed to send data");
    }

    return 0;
}

SERVMAK_NET_DEF int servmak_net_udp_socket_receive(servmak_net_socket_t *socket, servmak_net_address_t *sender, void *data, int size) {
    if (!socket) {
        return servmak_net__error("Socket is NULL");
    }

#ifdef _WIN32
    typedef int socklen_t;
#endif

    struct sockaddr_in from;
    socklen_t from_length = sizeof(from);

    int received_bytes = recvfrom(socket->handle, (char *) data, size, 0, (struct sockaddr *) &from, &from_length);
    if (received_bytes <= 0) {
        return 0;
    }

    sender->host = from.sin_addr.s_addr;
    sender->port = ntohs(from.sin_port);

    return received_bytes;
}

#endif //SERVMAK_NET_IMPLEMENTATION
