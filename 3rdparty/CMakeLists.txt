cmake_minimum_required(VERSION 2.8)

## 有意声明一个 project，主要是为了让后面的 CMAKE_SYSTEM 有值，CMake 要求 CMAKE_SYSTEM 必须在 project 之后。
project(3rdparty)

## 展开第三方依赖库直接编译进来，以简化安装和使用
if("${CMAKE_SYSTEM}" MATCHES "Linux")
    include(ExternalProject)
    set(3RDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    ExternalProject_Add(
        libxml2
        PREFIX ${3RDPARTY_DIR}/libxml2
        SOURCE_DIR ${3RDPARTY_DIR}/libxml2
        CONFIGURE_COMMAND ${3RDPARTY_DIR}/libxml2/configure --prefix=${3RDPARTY_DIR}/libxml2 --without-python --with-pic
        BUILD_COMMAND ${MAKE}
        )
    ExternalProject_Add(
        libuuid
        PREFIX ${3RDPARTY_DIR}/libuuid
        SOURCE_DIR ${3RDPARTY_DIR}/libuuid
        CONFIGURE_COMMAND ${3RDPARTY_DIR}/libuuid/configure --prefix=${3RDPARTY_DIR}/libuuid --with-pic
        BUILD_COMMAND ${MAKE}
        )
endif()

add_subdirectory(pcre)
if(WIN32)
    add_subdirectory(libiconv)
    add_subdirectory(libxml2/win32)
    add_subdirectory(c99-snprintf)
    add_subdirectory(zlib)
endif()
