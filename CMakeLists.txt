cmake_minimum_required(VERSION 3.12.0)
project(ServMak VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)


# Add the executable target
add_executable(ServMakCli 
example/client/main.cpp 
example/client/servmak_cli_impl.cpp
)

add_executable(ServMakCli_oop
example/client/main_oop.cpp 
example/client/servmak_cli_impl.cpp
)

add_executable(ServMakServ 
example/server/servmak_serv_impl.cpp 
example/server/main.cpp 
)


# Include the directories
target_include_directories(ServMakCli PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/example)
target_include_directories(ServMakCli_oop PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/example)
target_include_directories(ServMakServ PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/example)

# Link libraries for Windows
if(WIN32)
    target_link_libraries(ServMakCli wsock32 ws2_32)
    target_link_libraries(ServMakCli_oop wsock32 ws2_32)
    target_link_libraries(ServMakServ wsock32 ws2_32)
endif()
