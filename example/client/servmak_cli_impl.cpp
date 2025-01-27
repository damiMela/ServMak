/* 
 * The recommendation is to have the servmak implementation in a separate source file
 * 
 * it should contiain
 *      - the SERVMAK_CLIENT_IMPLEMENTATION macro 
 *      - the SERVMAK_STRUCT_RET_IMPLEMENT for the structs with return values
 *      - the SERVMAK_CALLBACK_IMPLEMENT for the callbacks
 * 
 * all of the above should be defined once in the hole code
 * 
 * The following is an example of what this file could look like
 * 
 * -------------------------------------------------------------------------------------------------
       
        #define SERVMAK_CLIENT_IMPLEMENTATION
        #include "serviceStructs.h"
        #include <servmak_lib/servmak_client.h>

        SERVMAK_STRUCT_RET_IMPLEMENT(myMethod);

        SERVMAK_CALLBACK_IMPLEMENT(myCallback, args,
        {
            printf("MY CUSTOM CALLBACK %d", args->notification);
        })

        SERVMAK_CALLBACK_IMPLEMENT(myUrgentCallback, args,
        {
            printf("MY URGENT CALLBACK. level %d. type %d", args->urgencyLevel, args->urgencyType);
        })

 * -------------------------------------------------------------------------------------------------
 *
*/


