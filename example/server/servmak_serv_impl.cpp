/* 
 * The recommendation is to have the servmak implementation in a separate source file
 * 
 * it should contiain
 *      - the SERVMAK_SERVER_IMPLEMENTATION macro 
 *      - the include to the cpp file (#include <path/to/servmak_server.cpp>)
 *      - the SERVMAK_STRUCT_ID_IMPLEMENT for all the structs. THIS SH
 * 
 * all of the above should be defined once in the hole code
 * 
 * The following is an example of what this file could look like
 * 
 * -------------------------------------------------------------------------------------------------
       
        #define SERVMAK_SERVER_IMPLEMENTATION
        #include <servmak_lib/servmak_server.hpp>

        #include "../serviceStructs.h"
        SERVMAK_STRUCT_ID_IMPLEMENT(myMethod)
        SERVMAK_STRUCT_ID_IMPLEMENT(myOtherMethod)
        SERVMAK_STRUCT_ID_IMPLEMENT(myCallback)
        
 * -------------------------------------------------------------------------------------------------
 *
*/




