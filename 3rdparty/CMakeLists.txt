cmake_minimum_required(VERSION 2.8.8)

## 展开第三方依赖库直接编译进来，以简化安装和使用
add_subdirectory(pcre)
if(WIN32)
    add_subdirectory(libiconv)
    add_subdirectory(libxml2/win32)
    add_subdirectory(c99-snprintf)
    add_subdirectory(zlib)
endif()
