cmake_minimum_required(VERSION 2.8.8)

if(WIN32)
    add_subdirectory(libiconv)
    add_subdirectory(pcre)
    add_subdirectory(libxml2)
    add_subdirectory(c99-snprintf)
    add_subdirectory(zlib)
endif()
