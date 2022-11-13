cmake_minimum_required(VERSION 3.13)
project(entry_task)

set(CMAKE_CXX_STANDARD 17)

find_library(ROCKSDB_LIB rocksdb)
message(STATUS "finding rocksdb lib ${ROCKSDB_LIB}")

find_path(ROCKSDB_INCLUDE_DIR rocksdb/db.h PATHS "")
include_directories(${ROCKSDB_INCLUDE_DIR})
message(STATUS "find rocksdb header ${ROCKSDB_INCLUDE_DIR}")

find_package(Threads REQUIRED)

add_executable(entry_task-server
        server_main.cpp server.cpp object.cpp threadpool.h file.cpp meta.cpp)
target_link_libraries(entry_task-server ${ROCKSDB_LIB} Threads::Threads)

add_executable(entry_task-client
        client_main.cpp object.cpp client.cpp file.cpp)
target_link_libraries(entry_task-client Threads::Threads)