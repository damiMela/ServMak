#ifndef SERVMAK_STRUCTS_H
#define SERVMAK_STRUCTS_H
#include <cinttypes>

struct servmak_struct_info{
  const uint8_t id;
  const uint8_t size;
  const uint8_t retSize;
};

#ifndef SERVMAK_STRUCT_INFO
#define SERVMAK_STRUCT_INFO(stName)  stName##_info
#endif 

#ifndef SERVMAK_STRUCT_ID
#define SERVMAK_STRUCT_ID(stName)  SERVMAK_STRUCT_INFO(stName).id
#endif 

#ifndef SERVMAK_STRUCT_LEN
#define SERVMAK_STRUCT_LEN(stName)  SERVMAK_STRUCT_INFO(stName).size
#endif 

#ifndef SERVMAK_STRUCT_RET_LEN
#define SERVMAK_STRUCT_RET_LEN(stName) SERVMAK_STRUCT_INFO(stName).retSize
#endif 

#ifndef SERVMAK_HANDLER
#define SERVMAK_HANDLER(stName) stName##_callbackHandle
#endif


#ifdef __cplusplus
  template <typename T>
  struct servmak_struct_id {
    static const uint8_t id;
  };

  #define SERVMAK_STRUCT_RET(stName, stId, retData, data)                                                             \
  typedef retData stName##_ret_t;                                                                                     \
  struct stName { data } __attribute__((packed));                                                                     \
  const servmak_struct_info SERVMAK_STRUCT_INFO(stName) = {.id=stId, .size=sizeof(stName), .retSize=sizeof(retData)}; 

  #define SERVMAK_STRUCT_VOID(stName, stId, data)                                                         \
  struct stName { data } __attribute__((packed));                                                         \
  const servmak_struct_info SERVMAK_STRUCT_INFO(stName) = {.id=stId, .size=sizeof(stName), .retSize=0};   
  
  
  #define SERVMAK_STRUCT_ID_IMPLEMENT(stName)         \
  template <> const uint8_t servmak_struct_id<stName>::id = SERVMAK_STRUCT_ID(stName);

  
#else
  #define SERVMAK_STRUCT_RET(stName, stId, retData, data)                                                             \
  typedef retData stName##_ret_t;                                                                                     \
  struct stName { data } __attribute__((packed));                                                                     \
  const servmak_struct_info SERVMAK_STRUCT_INFO(stName) = {.id=stId, .size=sizeof(stName), .retSize=sizeof(retData)}; 


  #define SERVMAK_STRUCT_VOID(stName, stId, data)                                                         \
  struct stName { data } __attribute__((packed));                                                         \
  const servmak_struct_info SERVMAK_STRUCT_INFO(stName) = {.id=stId, .size=sizeof(stName), .retSize=0}; 
#endif

#define SERVMAK_CALLBACK_DECLARE(stName)               void SERVMAK_HANDLER(stName)(void* argument)
#define SERVMAK_CALLBACK_DECLARE_OBJ(stName)           static void SERVMAK_HANDLER(stName)(void* argument)

#define SERVMAK_CALLBACK_IMPLEMENT(stName, argName, implementation)   \
    void SERVMAK_HANDLER(stName)(void* argument){stName* argName = (stName*)argument; implementation }

#define SERVMAK_CALLBACK_IMPLEMENT_OBJ(className, stName, argName, implementation)   \
    void className::SERVMAK_HANDLER(stName)(void* argument){stName* argName = (stName*)argument; implementation }


#endif