cmake_minimum_required(VERSION 3.3)


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
set(target_base64 base64)


# files
#######
file(GLOB src_b64 3rdparty/b64.c/*.c)
file(GLOB src_str src/str/*.c)
file(GLOB src_compress src/compress/*.c)
file(GLOB src_container src/container/*.c)
file(GLOB src_db src/db/*.c)
file(GLOB src_math src/math/*.c)
file(GLOB src_net src/net/*.c)
file(GLOB src_sys src/sys/*.c)
file(GLOB src_util src/util/*.c)

set(src_lib
    ${src_b64}
    3rdparty/mongoose_5.6/mongoose.c
    3rdparty/json_checker/JSON_checker.c
    3rdparty/CryptLib/lib/LibMd5.c
    ${src_str}
    ${src_compress}
    ${src_container}
    ${src_db}
    ${src_math}
    ${src_sys}
    ${src_util}
    )

set(src_base64
    test/unit/base64.c
    )

# include directory
include_directories(3rdparty)
include_directories(include)
# build options
###############
## home brew will install dependencies to /usr/local in default
if(APPLE)
    include_directories(/usr/local/include)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu")

## intend to set a blank link_flags in default to prevent error occurred
## in the latter call of set_target_properties
set(link_flags " ")

## build x86 universal(both i386 and x64) binary target for macOS
if(APPLE)
    set(c_flags "-arch i386 -arch x86_64")
    set(link_flags "-arch i386 -arch x86_64")
endif()
add_definitions(${c_flags})


# targets
#########
add_library(${target_static} STATIC ${src_lib})
add_library(${target_shared} SHARED ${src_lib})
add_executable(${target_base64} ${src_base64})

set_target_properties(${target_static} PROPERTIES
    OUTPUT_NAME ${lib_name_static})
set_target_properties(${target_shared} PROPERTIES
    OUTPUT_NAME ${lib_name}
    SOVERSION ${major_version}
    VERSION ${version}
    LINK_FLAGS ${link_flags}
    )

if(WIN32)
    set(link_libs )
elseif(APPLE)
    set(link_libs iconv dl xml2 curl pthread pcre)
elseif(UNIX)
    set(link_libs iconv dl xml2 curl pthread pcre uuid uv)
endif()

target_link_libraries(${target_shared} ${link_libs})
target_link_libraries(${target_base64} ${target_shared})
