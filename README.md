# ServMak
Local microservice maker

--------
## General
Servmak is a small collection of header files (4 files in total) that intents to make easier the development of microservice architecture for embedded systems.

Servmak allows both a request-reply behaviour (where the client "calls" a remote method) and, simultaneously, a callback behaviour where the server can asyncrouslly call a handler implemented in the client.

The server does not allow connections from any port, whenc reating the service, specific ports are allowed to establish a connection. this limits the memory footprint and allows for a better management of the internal services

* __Servmak_net__: "C" methods for using a UDP socket. 
* __Servmak_structs__: Macros that simplifies the definition and use of the methods used in the service
* __Servmak_server__: Class definitions and implementations that are used for a server/daemon creation. a C++11 compiler is required for this.
* __Servmak_client__: "C" methods and macros for calling remote methods and handling the callbacks.

## Usage
#### Structs/methods declarations
Both the server and the client needs a common structs that will serve the propuse of passing the arguments for the remote calls. 

An ID must be assigned for the struct. it should be an integer number until 253. this uniquely identifies the "remote method".

All the members of the struct should be "memcopiable". this macro declares a packed struct that is then copied as-is to a char* buffer (meaning that pointers or strings can not be used in this structs).

There are 2 macros for struct delcarations:
* __SERVMAK_STRUCT_RET__: this macro is used when the server returns a value after the call.
* __SERVMAK_STRUCT_VOID__: this macro is used when the client dos not expect a response from server

```c++
SERVMAK_STRUCT_VOID(thisIsTheStructName, METHOD_ID,
    uint32_t varA;
    time_t varB
    char varC[20];
);

//this struct returns a bool variable
SERVMAK_STRUCT_RET(myOtherMethod, METHOD_ID_2, bool,
    uint8_t var1;
    int16_t var2;
);
```

#### Server
When creating a server, in a soruce file, an implementation of the structs should be specified:
```c++
SERVMAK_STRUCT_ID_IMPLEMENT(thisIsTheStructName)
SERVMAK_STRUCT_ID_IMPLEMENT(myOtherMethod)
```

Minimal example:
```c++
SERVMAK_STRUCT_ID_IMPLEMENT(myOtherMethod)

myOtherMethod_ret_t myCustomMethodFn(myOtherMethod& args, uint16_t& client){
    std::cout << "calling custom method. Called from port "<<client << std::endl;
    return 55;
}

int main(){
    // the argument specifies the ports that can be used for clients
    ServmakServer server({8081,8082,8083});
    server.addMethod<myOtherMethod, myOtherMethod_ret_t>(myCustomMethodFn);
    
    if(! server.init(port) )
        return 1;

    while(true){
        server.loop();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    return 0;
}
```




## Compilation
`wsock32` library needed when compiling for a windows system.

The provided CMakeList automatically links the system libraries needed for socket usage.



### Notes
the file `Servmak_net.h` is mostly based in [zed_net library](https://github.com/Smilex/zed_net/)

for the example program [cmdline](https://github.com/tanakh/cmdline) library is used. (BSD-3-Clause license) 

