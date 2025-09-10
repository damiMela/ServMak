#include <cmdline.h>

#include <iostream>
#include <thread>
#include <chrono>

/*-------------------------------------------------------------------------*/

#define SERVMAK_SERVER_IMPLEMENTATION
#include <servmak_lib/servmak_server.hpp>

#include "../serviceStructs.h"
SERVMAK_STRUCT_ID_IMPLEMENT(myMethod)
SERVMAK_STRUCT_ID_IMPLEMENT(myOtherMethod)
SERVMAK_STRUCT_ID_IMPLEMENT(myCallback)

/*-------------------------------------------------------------------------*/


myMethod_ret_t myCustomMethodFn(myMethod& args, uint16_t& client){
    std::cout << "calling custom method. Called from port "<<client << ".  value:" << args.var1 << " str:" << args.str<< std::endl;
    return 'D';
}

void_ret_t  myCustomMethodFn2(myOtherMethod& args, uint16_t& client){
    std::cout << "calling custom method. Called from port "<<client << ".  value:" << args.var1 << " str:" << args.var1<< std::endl;
    return 0;
}

/*-------------------------------------------------------------------------*/


int server(uint16_t port){
    ServmakServer server({8081,8082,8083});
    if(! server.init(port) ){
        std::cout << "Could not init. exiting..." << std::endl;
        return 1;
    }

    server.addMethod<myMethod, myMethod_ret_t>(myCustomMethodFn);
    server.addMethod<myOtherMethod, void_ret_t>(myCustomMethodFn2);

    uint32_t cycleCount = 0;
    while(true){
        server.loop();
        std::this_thread::sleep_for(std::chrono::microseconds(100));

        cycleCount++; 
        if(cycleCount == 5000000){
            cycleCount = 0;
            std::cout << "sending callback" << std::endl;
            server.call<myCallback>({123456});
        }
    }

    return 0;
}

int main(int argc, char** argv){
    cmdline::parser cmdParser;
    cmdParser.add<int>("port", 'p', "port number",true);

    cmdParser.parse_check(argc, argv);

    uint16_t serverPort = cmdParser.get<int>("port");
    server(serverPort);
  
}