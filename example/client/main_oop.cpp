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

class ServmakClient{
public:
    ServmakClient(){}
    ~ServmakClient(){    servmak_cli_shutdown(&ctx);    }
    bool init(uint16_t serverPort, uint16_t clientPort); 
    void loop();
    int callMethod(uint8_t arg1, const std::string&& arg2);
    bool shouldQuit(){ return m_shouldQuit;}
public:
    SERVMAK_CALLBACK_DECLARE_OBJ(myCallback);
    SERVMAK_CALLBACK_DECLARE_OBJ(myUrgentCallback);

private:
    ServMak_cli_t ctx;
    static bool m_shouldQuit;
};

bool ServmakClient::m_shouldQuit = false;

bool ServmakClient::init(uint16_t serverPort, uint16_t clientPort){
    if( servmak_cli_init(&ctx, "127.0.0.1", serverPort, clientPort) >= 0)
        return false;

    //------>Register callBack methods
    servmak_register_callback(&ctx, myCallback);
    servmak_register_callback(&ctx, myUrgentCallback);
    return true;
    
}

void ServmakClient::loop(){
    servmak_cli_loop(&ctx);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

int ServmakClient::callMethod(uint8_t arg1, const std::string&& arg2){
    //------Execute a remote method
    myMethod myArgs = {arg1};
    strcpy(myArgs.str, arg2.c_str());

    myMethod_ret_t reply;
    if(servmak_request(&ctx, myMethod, &myArgs, &reply))
        std::cout << "GOT DATA OK:" << reply <<  std::endl;
    else
        std::cout << "GOT DATA FAIL" << std::endl;

    return reply;
}

SERVMAK_CALLBACK_IMPLEMENT_OBJ(ServmakClient, myCallback, args,
{
    static uint8_t counter = 0;
    printf("MY CUSTOM CALLBACK %d\n", args->notification);
    counter++;

    if(counter == 5){
        m_shouldQuit = true;
    }
})

SERVMAK_CALLBACK_IMPLEMENT_OBJ(ServmakClient,myUrgentCallback, args,
{
    printf("MY URGENT CALLBACK. level %d. type %d", args->urgencyLevel, args->urgencyType);
})

/*-------------------------------------------------------------------------*/
int client(uint16_t serverPort, uint16_t clientPort){
    std::cout << "starting at port " << clientPort << std::endl;


    //------Start servmak client
    ServmakClient client;
    if(!client.init(serverPort, clientPort)){
        std::cout << "failed to init client." << servmak_cli_get_error() << std::endl;
        return 1;
    }

    //------Call custom Method
    client.callMethod('X', "hola");
    
    //------Main Loop
    while(!client.shouldQuit()){
        client.loop();
    }


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