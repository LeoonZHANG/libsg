cmake_minimum_required(VERSION 2.6)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(CMAKE_C_FLAGS "-g -std=gnu99 -pthread -fpic")

link_directories(${PROJECT_SOURCE_DIR}/lib)
include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src/)
SET(SERVER_SOURCES
	tcp_server.c
)


ADD_EXECUTABLE(tcp_server ${SERVER_SOURCES})

TARGET_LINK_LIBRARIES(tcp_server uv pthread rt uvx)


SET(CLIENT_SOURCES
	tcp_client.c
)

ADD_EXECUTABLE(tcp_client ${CLIENT_SOURCES})
TARGET_LINK_LIBRARIES(tcp_client uv pthread rt uvx)


SET(UDPECHO_SOURCES
	udp-echo.c
)

ADD_EXECUTABLE(udpecho ${UDPECHO_SOURCES})
TARGET_LINK_LIBRARIES(udpecho uv pthread rt uvx)

SET(MULTI_CLIENT_SOURCES
	multiple_clients.c
)

ADD_EXECUTABLE(multi_clients ${MULTI_CLIENT_SOURCES})
TARGET_LINK_LIBRARIES(multi_clients uv pthread rt uvx)



