
#include "udp.h"
#include <string.h>
#ifdef __unix__
	#include <unistd.h>
#else
	#define sleep(n) Sleep((n)*1000)
#endif

// 
//  ./udpecho
//  ./udpecho 127.0.0.1


static void on_recv(uvx_udp_t* xudp, void* data, ssize_t datalen, const struct sockaddr* addr, unsigned int flag) {
    char ip[16]; int port; 

    cmm_get_ip_port(addr, ip, sizeof(ip), &port);
    printf("recv: %s  size: %zd  from %s:%d \n", (char *)data, datalen, ip, port);

    char *newdata = malloc(datalen); 
    memcpy(newdata, data, datalen);
    uvx_udp_send_to_addr(xudp, addr, newdata, datalen);
    sleep(1);
}

void main(int argc, char** argv) {
    char* target_ip = NULL;
    uv_loop_t* loop = uv_default_loop();
    uvx_udp_t xudp;
    uvx_udp_config_t config = uvx_udp_default_config(&xudp);
    config.on_recv = on_recv;

    if(argc > 1) target_ip = argv[1];
    if(target_ip == NULL) {
        uvx_udp_start(&xudp, loop, "0.0.0.0", 8008, config); 
    } else {
        uvx_udp_start(&xudp, loop, NULL, 0, config); 
	int send_size = strlen("Hello echo")+1;
        char *send_data = malloc(send_size);
	strcpy(send_data, "Hello echo");
	uvx_udp_send_to_ip(&xudp, target_ip, 8008, send_data, send_size); 
    }

    uv_run(loop, UV_RUN_DEFAULT);
}
