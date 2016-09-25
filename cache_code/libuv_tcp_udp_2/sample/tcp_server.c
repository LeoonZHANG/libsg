
#include "tcp_server.h"

unsigned long total_bytes = 0;                  //Total bytes tcp server received
unsigned long last_total_bytes = 0;             
unsigned long print_single_connection = 0;      //
/*
Connections tcp server configure, if the system file discriptor limits is smaller than this, 
system file discriptor limits should be enlarged firstly.
*/
unsigned long total_connections = 1024;         
static void on_conn_ok(tcp_server_t* xserver, tcp_server_conn_t* conn) {
    char ip[40]; 
    int port;

    //使用conn句柄中的extra字段来记录自己的信息, 本程序用来记录每条连接上收到的数据
    unsigned long *tmp = (unsigned long *)conn->extra;
    *tmp = 0;

    //获取客户端的IP地址和端口号
    cmm_get_tcp_ip_port(&conn->uvclient, ip, sizeof(ip), &port);

    //获取当前server已建立的连接数量, 遍历回调为NULL时, 只获取连接数量
    int connections = tcp_server_iter_conns(xserver, NULL, NULL);
    
    printf("receive new connection from [%s:%d], current total_connections = %d\n", 
            ip, port, connections);
}


static void on_recv(tcp_server_t* xserver, tcp_server_conn_t* conn, void* data, ssize_t datalen) {

    /* 统计收到的数据 */
    unsigned long *tmp = (unsigned long *)conn->extra;
    *tmp += datalen;
    total_bytes += datalen;
}

static void on_conn_closing(tcp_server_t* xserver, tcp_server_conn_t* conn) {
    char ip[40]; int port;
    cmm_get_tcp_ip_port(&conn->uvclient, ip, sizeof(ip), &port);
    printf("client [%s:%d] on_conn_closing\n", ip, port);
}

static void on_conn_close(tcp_server_t* xserver, tcp_server_conn_t* conn) {
}
void on_iter_connection(tcp_server_t* xserver, tcp_server_conn_t* conn, void* userdata)
{
    char ip[40]; 
    int port;
    unsigned long *tmp = (unsigned long *)conn->extra;
    cmm_get_tcp_ip_port(&conn->uvclient, ip, sizeof(ip), &port);
    if (print_single_connection)
    {
        printf("Receive %lu bytes from client [%s:%d] \n", *tmp, ip, port); 
    }
}


static void  on_heartbeat(tcp_server_t* xserver, unsigned int index) {
    

    int hb = (int)xserver->config.heartbeat_interval_seconds;

    
    /* 使用on_iter_connection函数遍历已有的所有连接 */
    printf("********************************************************************\n");
    int connections = tcp_server_iter_conns(xserver, on_iter_connection, NULL);
    printf("[Statistic]\n");
    printf("Total Connections                               = %d\n", connections);
    printf("Total Bytes Received                            = %lu\n", total_bytes);
    printf("Bytes Received per seconds                      = %lu\n", (unsigned long)(total_bytes/(index*hb)));    
    printf("Bytes Received In Last %d seconds               = %lu\n", hb, (unsigned long)(total_bytes - last_total_bytes));
    printf("Bytes Received per seconds In Last %d seconds   = %lu\n", hb, (unsigned long)((total_bytes - last_total_bytes)/hb));
    printf("*******************************************************************\n"); 
    last_total_bytes = total_bytes;       
}

int hb_interval = 30;
void main(int argc, char *argv[]) {

    if(argc > 1) {
        hb_interval = atoi(argv[1]);
    }

    if(argc > 2) {
        total_connections = (unsigned int)atoi(argv[2]);
    }

    if(argc > 3) {
        print_single_connection = (unsigned int)atoi(argv[3]);
    }
    

    uv_loop_t* loop = uv_default_loop();
    tcp_server_t server;
    tcp_server_config_t config = tcp_server_default_config(&server);
    config.conn_count = total_connections;                   //tcp server 靠靠靠靠
    config.on_conn_ok = on_conn_ok;
    config.on_recv = on_recv;
    config.on_conn_closing = on_conn_closing;
    config.on_conn_close = on_conn_close;
    config.on_heartbeat = on_heartbeat;
    config.conn_extra_size = 8;
    config.heartbeat_interval_seconds = (float)hb_interval;  //设置server心跳时间
    
    tcp_server_start(&server, loop, "127.0.0.1", 8001, config);
    uv_run(loop, UV_RUN_DEFAULT);
}
