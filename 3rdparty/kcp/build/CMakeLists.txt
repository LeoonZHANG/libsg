cmake_minimum_required(VERSION 2.6)

PROJECT(kcp)

SET(CMAKE_C_FLAGS "-O3 -DDEBUG -std=gnu99 -pthread -fpic -g")

SET(KCP_SOURCES 
	../ikcp.c
)

ADD_LIBRARY(kcp ${KCP_SOURCES})

#TARGET_LINK_LIBRARIES(kcp uv pthread rt)