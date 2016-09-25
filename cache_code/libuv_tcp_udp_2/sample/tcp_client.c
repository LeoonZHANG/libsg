
#include "tcp.h"


// Usage:
//   ./tcp_client         Send 1 data  per 10 seconds
//   ./tcp_client x       Send x datas per 10 seconds
//   ./tcp_client x y     Send x datas per y seconds

int bench = 1;
int interval = 10; // Seconds

static void on_heartbeat(tcp_client_t* xclient, unsigned int index) {
    static char data[] = {'u','v','x','-','s','e','r','v','e','r',',',
                          'b','y',' ','w','e','i','w','e','i','\n'};
    printf("send %d datas\n", bench);

    char *tmp = NULL;
    for(int i = 0; i < bench; i++) {
        tmp = malloc(sizeof(data));
        memcpy(tmp, data, sizeof(data));
        tcp_client_send(xclient, tmp, sizeof(data));
    }
}

void main(int argc, char** argv) {
    if(argc > 1) {
        bench = atoi(argv[1]);
    }
    if(argc > 2) {
        interval = atoi(argv[2]); // Seconds
    }

    uv_loop_t* uvloop = uv_default_loop();
    tcp_client_t client;
    tcp_client_config_t config = tcp_client_default_config(&client);
    config.on_heartbeat = on_heartbeat;
    config.heartbeat_interval_seconds = (float)interval;
    tcp_client_connect(&client, uvloop, "127.0.0.1", 8001, config);

    uv_run(uvloop, UV_RUN_DEFAULT);
}
