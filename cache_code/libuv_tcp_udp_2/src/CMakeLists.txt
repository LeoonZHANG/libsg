cmake_minimum_required(VERSION 2.6)

#
aux_source_directory(. srcs0)
aux_source_directory(./utils/ srcs1)
set(srcs ${srcs0} ${srcs1})

include_directories(${PROJECT_SOURCE_DIR}/include ./)


# Set library output path
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# shared lib
add_library(uvx SHARED ${srcs})
# static lib
add_library(uvx_static STATIC ${srcs})
 
# change static lib name to uvx
set_target_properties(uvx_static PROPERTIES OUTPUT_NAME "uvx")
# make static & shared lib together
set_target_properties(uvx PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(uvx_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
 
# Set version of shared lib version
# VERSION 
# SOVERSION 
set_target_properties(uvx PROPERTIES VERSION 1.0 SOVERSION 1)

TARGET_LINK_LIBRARIES(uvx uv pthread rt)
