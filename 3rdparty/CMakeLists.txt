cmake_minimum_required(VERSION 2.8.8)

## 展开第三方依赖库直接编译进来，以简化安装和使用，这样主要是针对像Windows这种没有包管理的系统。
## Linux 和 macOS 还是主要使用包管理程序安装依赖包。
if(WIN32)
    add_subdirectory(libiconv)
    add_subdirectory(pcre)
    add_subdirectory(libxml2/win32)
    add_subdirectory(c99-snprintf)
    add_subdirectory(zlib)
endif()
