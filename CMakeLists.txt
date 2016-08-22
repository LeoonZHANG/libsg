cmake_minimum_required(VERSION 3.3)

add_subdirectory(3rdparty)


# project
#########
project(libsg)

## Version number is quiet important for *nix dynamic lib, specify here.
set(major_version 1)
set(minor_version 0)
set(patch_version 0)
set(build_version 0)
set(version ${major_version}.${minor_version}.${patch_version})

set(lib_name sg)
set(lib_name_static ${lib_name})
## Take different names for static lib and dynamic lib on Windows, or they're .lib file will conflicted.
if(WIN32)
    set(lib_name_static lib${lib_name_static})
endif()
set(target_static lib${lib_name}-static)
set(target_shared lib${lib_name}-shared)
set(copyright "Copyright (C) 2016." CACHE STRING "copyright description")


# files
#######
file(GLOB src_b64 3rdparty/b64.c/*.c)
file(GLOB src_cjson 3rdparty/cjson/*.c)
file(GLOB src_vtd-xml 3rdparty/vtd-xml/*.c)
## exclude unecessary "lex.yy (2).c" and "lex.yy_old.c" to prevent build failed
list(REMOVE_ITEM src_vtd-xml
    "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/vtd-xml/lex.yy (2).c"
    "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/vtd-xml/lex.yy_old.c")
file(GLOB src_str src/str/*.c)
file(GLOB src_compress src/compress/*.c)
file(GLOB src_container src/container/*.c)
file(GLOB src_db src/db/*.c)
file(GLOB src_math src/math/*.c)
file(GLOB src_net src/net/*.c)
file(GLOB src_platform src/platform/windows/*.c)
file(GLOB src_sys src/sys/*.c)
file(GLOB src_util src/util/*.c)

set(src_lib
    ${src_b64}
    ${src_cjson}
    3rdparty/mongoose_5.6/mongoose.c
    3rdparty/json_checker/JSON_checker.c
    3rdparty/CryptLib/lib/LibMd5.c
    ${src_str}
    ${src_compress}
    ${src_container}
    ${src_db}
    ${src_math}
    ${src_platform}
    ${src_sys}
    ${src_util}
    )

## Use installed lib on Linux, compile from source is required on other platforms
if(NOT "${CMAKE_SYSTEM}" MATCHES "Linux")
    set(src_lib ${src_lib} 
        ${src_vtd-xml}
        )
endif()

if(WIN32)
    configure_file(version.rc.in version.rc @ONLY)
    configure_file(module.def.in module.def @ONLY)
    set(src_lib ${src_lib} module.def version.rc)
endif()


# build options
###############
## home brew will install dependencies to /usr/local in default
if(APPLE)
    include_directories(
		/usr/local/include
		/usr/local/include/libxml2
		)
endif()
include_directories(3rdparty)
include_directories(include)
if(WIN32)
    include_directories(3rdparty/libiconv/include)
    include_directories(3rdparty/pcre)
    include_directories(3rdparty/libxml2/include)
    include_directories(3rdparty/zlib)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
if("${CMAKE_SYSTEM}" MATCHES "Linux")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu")
endif()

## intend to set a blank link_flags in default to prevent error occurred
## in the latter call of set_target_properties
set(link_flags " ")

## build x86 universal(both i386 and x64) binary target for macOS
if(APPLE)
    set(c_flags "-arch i386 -arch x86_64")
    set(link_flags "-arch i386 -arch x86_64")
endif()
add_definitions(${c_flags})
if(WIN32)
    add_definitions(-DUSING_STATIC_LIBICONV -DPCRE_STATIC)
endif()


# targets
#########
add_library(${target_static} STATIC ${src_lib})
add_library(${target_shared} SHARED ${src_lib})

set_target_properties(${target_static} PROPERTIES
    OUTPUT_NAME ${lib_name_static})
set_target_properties(${target_shared} PROPERTIES
    OUTPUT_NAME ${lib_name}
    SOVERSION ${major_version}
    VERSION ${version}
    LINK_FLAGS ${link_flags}
    )

if(WIN32)
    set(libxml-static-file ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libxml2/win32/bin.msvc/libxml2_a.lib)
    set(link_libs libiconv-static pcre ${libxml-static-file} libc99-snprintf-static zlibstatic)
elseif(APPLE)
    set(link_libs iconv dl xml2 curl pthread pcre z)
elseif(UNIX)
    set(link_libs iconv dl xml2 curl pthread pcre uuid uv)
endif()

target_link_libraries(${target_shared} ${link_libs})
target_link_libraries(${target_static} ${link_libs})

add_subdirectory(test/unit)
