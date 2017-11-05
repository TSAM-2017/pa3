cmake_minimum_required(VERSION 3.8)
project(c_http_server)

set(CMAKE_C_STANDARD 11)

#include_directories(/usr/local/opt/openssl/include)
#link_directories(/usr/local/opt/openssl/lib)

#include_directories(/usr/local/include/glib-2.0)
#link_directories(/usr/local/lib)

set(SOURCE_FILES
        src/main.c
        src/http_spec.h
        src/http_server.h src/http_server.c
        src/http_get.h src/http_get.c
        src/http_head.h src/http_head.c
        src/http_post.h src/http_post.c)
add_executable(c_http_server ${SOURCE_FILES})

#target_link_libraries(c_http_server ssl)
#target_link_libraries(c_http_server crypto)

#target_link_libraries(c_http_server glib-2.0.dylib)