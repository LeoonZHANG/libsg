


#include "../include/tcp.h"


// Usage:
//   ./multi_client         10 connections to server, 100 bytes per second
//   ./mutli_client x       x connections to server, 100 bytes per second per connection
//   ./mutli_client x y     x connections to server, y bytes per second per connection

int clients = 10;
int bytes = 100; 
unsigned long total_bytes_sent  = 0;
static void on_heartbeat(tcp_client_t* xclient, unsigned int index) {
    char *tmp = NULL;

    tmp = malloc(sizeof(bytes));
    tcp_client_send(xclient, tmp, bytes);

    total_bytes_sent +=  bytes;
}

static void on_print_timer(uv_timer_t* handle){
    printf("************************************************\n");
    printf("Total Bytes Sent:       %lu\n", total_bytes_sent);
    printf("************************************************\n");
}


void main(int argc, char** argv) {

 
    if(argc > 1) {
        clients = atoi(argv[1]);
    }
    if(argc > 2) {
        bytes = atoi(argv[2]); 
    }

    uv_loop_t* uvloop = uv_default_loop();

    //生成clients 
    tcp_client_config_t config;
    tcp_client_t *client = 0; 
    for (int i = 0; i < clients; i++)
    {
        client = malloc(sizeof(tcp_client_t));
        config = tcp_client_default_config(client);
        config.on_heartbeat = on_heartbeat;
        config.heartbeat_interval_seconds = (float)1;
        tcp_client_connect(client, uvloop, "127.0.0.1", 8001, config);
    }

    //单独启动一个uv定时器, 用于发送端数据周期统计打印
    uv_timer_t print_timer;
    uv_timer_init(uvloop, &print_timer);
    uv_timer_start(&print_timer, on_print_timer, 10000, 10000);
    
    uv_run(uvloop, UV_RUN_DEFAULT);
}


