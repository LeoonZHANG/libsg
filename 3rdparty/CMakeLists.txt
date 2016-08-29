cmake_minimum_required(VERSION 2.8.8)

## 展开第三方依赖库直接编译进来，以简化安装和使用
if(NOT "${CMAKE_SYSTEM}" MATCHES "Linux")
    include(ExternalProject)
    set(3RDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    ExternalProject_Add(
        libxml2
        PREFIX ${3RDPARTY_DIR}/libxml2
        SOURCE_DIR ${3RDPARTY_DIR}/libxml2
        CONFIGURE_COMMAND ${3RDPARTY_DIR}/libxml2/configure --prefix=${3RDPARTY_DIR}/libxml2 --without-python --with-pic
        BUILD_COMMAND ${MAKE}
        LOG_CONFIGURE 1
        LOG_BUILD 1
        )
endif()

add_subdirectory(pcre)
if(WIN32)
    add_subdirectory(libiconv)
    add_subdirectory(libxml2/win32)
    add_subdirectory(c99-snprintf)
    add_subdirectory(zlib)
endif()
