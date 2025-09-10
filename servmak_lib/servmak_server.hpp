#pragma once

#if defined(SERVMAK_SERVER_IMPLEMENTATION) && !defined(SERVMAK__OMIT_NET_IMPLEMETATION)
#define SERVMAK_NET_IMPLEMENTATION
#endif

#ifdef SERVMAK_CUSTOM_NET_LOCATION
#include SERVMAK_CUSTOM_NET_LOCATION
#else
#include "servmak_net.h"
#endif 

#include "servmak_structs.h"

#include <cinttypes>
#include <vector>
#include <queue>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <unordered_map>
#include <algorithm>
#include <exception>
#include <iostream>

typedef uint8_t void_ret_t;

/*---------------------------------------------------------------------*/

/// @brief Thread Safe Queue
/// @tparam T 
template <class T>
class ServmakQueue{
public:
  ServmakQueue(void) : m_queue(), m_mtx(), m_condVar() {}
  ~ServmakQueue(void){}

  void enqueue(T var){
    std::lock_guard<std::mutex> lock(m_mtx);
    m_queue.push(var);
    m_condVar.notify_one();
  }

  T dequeue(void){
    std::unique_lock<std::mutex> lock(m_mtx);
    while(m_queue.empty()){
      m_condVar.wait(lock);
    }
    T val = m_queue.front();
    m_queue.pop();
    return val;
  }

  bool pending(){
    std::unique_lock<std::mutex> lock(m_mtx);
    return !m_queue.empty();
  }

private:
  std::queue<T> m_queue;
  mutable std::mutex m_mtx;
  std::condition_variable m_condVar;
};

/*---------------------------------------------------------------------*/
class IServmakHandler{
public:
  virtual ~IServmakHandler() = default;
  virtual bool call(std::vector<uint8_t> buffer, uint16_t client) = 0;
  virtual uint8_t size() = 0;
  virtual std::vector<uint8_t> result() = 0;
};

template<typename T, typename R>
class ServmakHandler : public IServmakHandler{
public:
  ServmakHandler(std::function<R(T&, uint16_t&)> handler) : m_handler(handler){}
  virtual ~ServmakHandler() = default;

  bool call(std::vector<uint8_t> buffer, uint16_t client) override{
    if(buffer.size() != sizeof(T))
      return false;
    
    T arguments;
    memcpy(&arguments, buffer.data(), sizeof(T));
    m_result = m_handler(arguments, client);
    return true;
  }

  std::vector<uint8_t> result() override {
    std::vector<uint8_t> resBuffer(sizeof(R));
    memcpy(resBuffer.data(), &m_result, sizeof(R));
    return resBuffer;
  }

  uint8_t size(){
    return sizeof(T);
  }

private:
    std::function<R(T&, uint16_t&)> m_handler;
    R m_result;
};

/*---------------------------------------------------------------------*/

class IServmakCallback{
public:
  virtual ~IServmakCallback() = default;

};
/*---------------------------------------------------------------------*/

class ServmakServer{
public:
    ServmakServer(std::initializer_list<uint16_t> clientPorts){
      if(clientPorts.size() > 0xFF)
        throw std::runtime_error("The maximum amount of clients is 255. Can not create server");

      uint8_t i = 0;
      for(const auto& clientPort : clientPorts){
          servmak_net_get_address(&m_clients[i], "127.0.0.1", clientPort);
          i++;
      }
    }
    ~ServmakServer(){
      servmak_net_socket_close(&m_socket);
      servmak_net_shutdown();
    }

    bool init(const uint16_t& port ){
        if(servmak_net_init() < 0){
            std::cout << "Failed to init \"zed net\". " << servmak_net_get_error() << std::endl;
            return false;
        }
        if (servmak_net_udp_socket_open(&m_socket, port, 1) < 0) {
            std::cout << "Failed to open socket. "<< servmak_net_get_error()  << std::endl;
            return false;
        }
        return true;
    }
    void loop(){
      servmak_net_address_t sender;
      std::vector<uint8_t> buff(255, 0); //buffer of 50 bytes

      if(m_queue.pending()){
          auto pkg = m_queue.dequeue();
          servmak_net_udp_socket_send(&m_socket, pkg.client, pkg.buff.data(), pkg.buff.size());
      }

      int bytes_read = servmak_net_udp_socket_receive(&m_socket, &sender, buff.data(), buff.size());
      if(bytes_read < 1)
          return;

      auto client = std::find_if(m_clients.begin(), m_clients.end(), [&sender](const servmak_net_address_t& client){return client.port == sender.port;}); 
      if(client == m_clients.end()){
          std::cout << "Pkg recieved from unouthorized client" << std::endl;
          return;
      }

      uint8_t& methodId = buff[0];
      if(m_methods.count(methodId) != 1){
          std::cout << "Pkg received with invalid method ID:" << methodId<< std::endl;
          clientPkg pkg = {*client, sec_errorInvalidMethod};
          m_queue.enqueue(pkg);
          return;
      }
      
      if((bytes_read-1) != m_methods[methodId]->size()){
          std::cout << "Pkg received with invalid size. ID:" << (uint16_t)methodId << ". size:" <<(uint16_t)bytes_read << ". expected:" << (uint16_t)m_methods[methodId]->size() << std::endl;
          clientPkg pkg = {*client, sec_errorInvalidSize};
          m_queue.enqueue(pkg);
          return;
      }

      std::vector<uint8_t> pkgData(buff.begin()+1, buff.begin()+bytes_read);
      if(m_methods[methodId]->call(pkgData, sender.port)){
          clientPkg pkg = {*client, m_methods[methodId]->result()};
          m_queue.enqueue(pkg);
      }
    }

    template<typename T, typename R>
    void addMethod(std::function<R(T&, uint16_t&)> handler){
      auto _handler =  std::make_shared<ServmakHandler<T, R>>(handler);
      m_methods.emplace(servmak_struct_id<T>::id, _handler);
    }

    template<typename T>
    void addMethod(std::function<void(T&, uint16_t&)> handler){
      auto _handler =  std::make_shared<ServmakHandler<T, void_ret_t>>(handler);
      m_methods.emplace(servmak_struct_id<T>::id, _handler);
    }


    template<typename T>
    void call(const T&& values){
      for(auto& client : m_clients){
        std::vector<uint8_t> buff(sizeof(T)+1);
        buff[0] = servmak_struct_id<T>::id;
        memcpy(buff.data()+1, &values, sizeof(T));
        clientPkg pkg = {client, buff};
        m_queue.enqueue(pkg);
      }
    }

private:
  struct clientPkg{
    servmak_net_address_t& client;
    std::vector<uint8_t> buff;
  };
private:
    const std::vector<uint8_t> sec_newClient = {0xFE, 0xAA, 0x55};
    const std::vector<uint8_t> sec_errorInvalidMethod = {0xFE, 0xE0};
    const std::vector<uint8_t> sec_errorInvalidSize = {0xFE, 0xE0};
    
private:
    ServmakQueue<clientPkg> m_queue;
    servmak_net_socket_t m_socket;
    std::vector<servmak_net_address_t> m_clients;
    std::unordered_map<uint8_t, std::shared_ptr<IServmakHandler>> m_methods;
    std::unordered_map<uint8_t, std::shared_ptr<IServmakCallback>> m_callbacks;
};

