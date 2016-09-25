简介
基于Libuv完成的TCP/UDP功能封装组件, 基于本封装组件, 应用层可以快速的写出高并发tcp/udp程序

目录结构和文件介绍
include      对外提供的头文件目录
src		     封装源代码文件目录
sample		 样例代码文件目录
bin		     样例代码生成的二进制目录
lib			 封装源代码生成库文件目录
build		 CMake 专用build目录

使用帮助(Ubuntu 14.04)
1) 安装build工具链
apt-get update
apt-get install build-essential cmake

2) 安装libuv库
参见libuv官方文档完成libuv库编译安装 https://github.com/libuv/libuv/blob/master/README.md

3) 编译本框架代码
3.1) 将框架代码上传到Linux系统
3.2) 执行下述命令
     cd build/
	 cmake ..
	 make
	 
上述命令完成后, 在lib目录下会自动生成libuvx.a和libuvx.so以及相关软链接, 在bin目录下会生成multi_clients/tcp_client/tcp_server/udpecho四个程序;

样例文件介绍:
tcp_server.c       介绍了如何使用本框架快速完成1个高并发tcp server开发, 可与tcp_client.c multi_clients.c进行对接验证
tcp_client.c       介绍了如何使用本框架快速完成单个tcp client链接开发
multi_clients.c    介绍了如何使用本框架快速完成多个tcp client链接开发
udpecho.c          介绍了如何使用本框架快速完成udp节点通信开发

样例命令参数介绍:
tcp_server  x  y        x是x秒进行一次屏幕打印, 输出当前tcp server的连接数量和接收数据信息; y为tcp_server最大支持连接数量
tcp_client  x  y        x是指每次发送多少数据;  y是指多少秒发送一次数据;
multi_clients x y       x是指每条连接每秒发送多少数据;  y是指与server建立连接的数量;
udpecho					这是一个udp实例, 显示从对方udp收到的信息, 并回传给对方
udp A: ./udpecho         等待接收UDP报文, 收到后显示在屏幕上, 并将信息原样发回;
udp B: ./udpecho 127.0.0.1  发送Hello echo给本机的udpecho, 并等待接收UDP报文, 收到后显示在屏幕上, 并将信息原样发回;

