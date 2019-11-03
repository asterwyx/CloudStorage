cmake_minimum_required(VERSION 3.15)
project(CloudStorageServer C)

set(CMAKE_C_STANDARD 11)

add_executable(CloudStorageServer cloudStorageServer.c http.h util.h http.c util.c threadpool.c threadpool.h condition.c condition.h eventCtl.h eventCtl.c)
add_executable(CloudStorageClient cloudStorageClient.c http.h util.h http.c util.c threadpool.c threadpool.h condition.c condition.h eventCtl.h eventCtl.c)
add_executable(TestServer testServer.c)
add_executable(Test test.c http.h http.c util.h util.c)
find_package(Threads REQUIRED)
target_link_libraries(CloudStorageServer Threads::Threads)
target_link_libraries(CloudStorageClient Threads::Threads)