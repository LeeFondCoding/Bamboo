cmake_minimum_required(VERSION 3.0)

project(bamboo CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE DEBUG)

file(GLOB BASE_FILES net/base/*.cc)

file(GLOB NET_FILES net/net/*.cc)

file(GLOB DB_FILES db/*.cc)

file(GLOB MANAGER_FILES controller/*.cc)

include_directories(net .)

add_subdirectory(test)

find_package(leveldb REQUIRED)

add_executable(Server common/Server.cc ${BASE_FILES} ${NET_FILES} ${MANAGER_FILES})
target_link_libraries(Server leveldb)

add_executable(Client common/Client.cc ${BASE_FILES} ${NET_FILES} ${MANAGER_FILES})
target_link_libraries(Client leveldb)

add_executable(test_async_logging test/net/base/test_async_logging.cc ${BASE_FILES})
