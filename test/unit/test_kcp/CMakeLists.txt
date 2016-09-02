cmake_minimum_required(VERSION 2.6)

PROJECT(ckcpuv)

SET(CMAKE_C_FLAGS "-O3 -DDEBUG -std=gnu99 -pthread -fpic -Wall -g")

SET(LIBSG_DIR ../../..)

SET(LIBUV_DIR ${LIBSG_DIR}/3rdparty/libuv/lib/linux_x64)
SET(NET_DIR ${LIBSG_DIR}/src/net)
SET(KCP_DIR ${LIBSG_DIR}/3rdparty/kcp)
SET(LINKHASH_DIR ${LIBSG_DIR}/3rdparty/linkhash)

INCLUDE_DIRECTORIES(
    ${LIBSG_DIR}/include/sg/net
    ${LIBSG_DIR}/3rdparty/kcp
    ${LIBSG_DIR}/3rdparty/libuv/inc
    ${LINKHASH_DIR}
)

SET(SRC_SERVER
    ${KCP_DIR}/ikcp.c
    ${LINKHASH_DIR}/linkhash.c
    ${NET_DIR}/etp_server.c
    ../kcp_server.c
)

SET(SRC_CLIENT
    ${KCP_DIR}/ikcp.c
    ${NET_DIR}/etp.c
    ../kcp_client.c
)

LINK_DIRECTORIES(
    ${CKCPUV_DIR}/libuv/lib/linux_x64
)

ADD_EXECUTABLE(kcp_server ${SRC_SERVER})
TARGET_LINK_LIBRARIES(kcp_server uv)

ADD_EXECUTABLE(kcp_client ${SRC_CLIENT})
TARGET_LINK_LIBRARIES(kcp_client uv)
