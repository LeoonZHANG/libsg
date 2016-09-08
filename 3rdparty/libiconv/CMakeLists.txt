cmake_minimum_required(VERSION 2.8)


# project
#########
project(libiconv)

set(lib_name iconv)
set(lib_name_static ${lib_name})
## Take different names for static lib and dynamic lib on Windows, or they're .lib file will conflicted.
if(WIN32)
    set(lib_name_static lib${lib_name_static})
endif()
set(target_static lib${lib_name}-static)


# files
#######
file(GLOB_RECURSE src_lib src/*.c)


# build options
###############
include_directories(include)
add_definitions(-DICONV_CONST=const -DBUILDING_LIBICONV -DBUILDING_LIBCHARSET)


# targets
#########
add_library(${target_static} STATIC ${src_lib})

set_target_properties(${target_static} PROPERTIES
    OUTPUT_NAME ${lib_name_static})
