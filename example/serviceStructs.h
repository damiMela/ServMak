#include <servmak_lib/servmak_structs.h>
#define SERVMAK_CLI_CALLBACK_COUNT 2
/*--------------------------------------------------------*/
enum e_structsId{
    MY_METHOD_ID = 1,
    MY_OTHER_METHOD_ID,
    MY_CALLBACK_ID = 100,
    MY_URGENT_CALLBACK_ID
};

/*--------------------------------------------------------*/
/*
 * The methods that expect the server to send somethin back to the client should be of type SERVMAK_STRUCT_RET
 * those methods declared as STRUCT_VOID will send the request and will not wait for a reply
 * 
 * All the members of the struct should be "memcopiable". this macro declares a packed struct that is then 
 * copied as-is to a char* buffer (meaning that pointers or strings can not be used in this structs)
 * 
*/
SERVMAK_STRUCT_RET(myMethod, MY_METHOD_ID, int,
    uint8_t var1;
    char str[5];
);

SERVMAK_STRUCT_VOID(myOtherMethod, MY_OTHER_METHOD_ID,
    uint8_t var1;
    uint16_t var2;
);


/*--------------------------------------------------------*/
/*
 * All callback structure declarations must be of type STRUCT_VOID
 * The server will not expect a response from the clients should it send a message
 * 
 * All the members of the struct should be "memcopiable". this macro declares a packed struct that is then 
 * copied as-is to a char* buffer (meaning that pointers or strings can not be used in this structs)
 * 
*/
SERVMAK_STRUCT_VOID(myCallback, MY_CALLBACK_ID,
    uint32_t notification;
);

SERVMAK_STRUCT_VOID(myUrgentCallback, MY_URGENT_CALLBACK_ID,
    uint8_t urgencyLevel;
    uint16_t urgencyType;
);

/*--------------------------------------------------------*/
