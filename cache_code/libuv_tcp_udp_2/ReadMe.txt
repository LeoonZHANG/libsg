���
����Libuv��ɵ�TCP/UDP���ܷ�װ���, ���ڱ���װ���, Ӧ�ò���Կ��ٵ�д���߲���tcp/udp����

Ŀ¼�ṹ���ļ�����
include      �����ṩ��ͷ�ļ�Ŀ¼
src		     ��װԴ�����ļ�Ŀ¼
sample		 ���������ļ�Ŀ¼
bin		     �����������ɵĶ�����Ŀ¼
lib			 ��װԴ�������ɿ��ļ�Ŀ¼
build		 CMake ר��buildĿ¼

ʹ�ð���(Ubuntu 14.04)
1) ��װbuild������
apt-get update
apt-get install build-essential cmake

2) ��װlibuv��
�μ�libuv�ٷ��ĵ����libuv����밲װ https://github.com/libuv/libuv/blob/master/README.md

3) ���뱾��ܴ���
3.1) ����ܴ����ϴ���Linuxϵͳ
3.2) ִ����������
     cd build/
	 cmake ..
	 make
	 
����������ɺ�, ��libĿ¼�»��Զ�����libuvx.a��libuvx.so�Լ����������, ��binĿ¼�»�����multi_clients/tcp_client/tcp_server/udpecho�ĸ�����;

�����ļ�����:
tcp_server.c       ���������ʹ�ñ���ܿ������1���߲���tcp server����, ����tcp_client.c multi_clients.c���жԽ���֤
tcp_client.c       ���������ʹ�ñ���ܿ�����ɵ���tcp client���ӿ���
multi_clients.c    ���������ʹ�ñ���ܿ�����ɶ��tcp client���ӿ���
udpecho.c          ���������ʹ�ñ���ܿ������udp�ڵ�ͨ�ſ���

���������������:
tcp_server  x  y        x��x�����һ����Ļ��ӡ, �����ǰtcp server�����������ͽ���������Ϣ; yΪtcp_server���֧����������
tcp_client  x  y        x��ָÿ�η��Ͷ�������;  y��ָ�����뷢��һ������;
multi_clients x y       x��ָÿ������ÿ�뷢�Ͷ�������;  y��ָ��server�������ӵ�����;
udpecho					����һ��udpʵ��, ��ʾ�ӶԷ�udp�յ�����Ϣ, ���ش����Է�
udp A: ./udpecho         �ȴ�����UDP����, �յ�����ʾ����Ļ��, ������Ϣԭ������;
udp B: ./udpecho 127.0.0.1  ����Hello echo��������udpecho, ���ȴ�����UDP����, �յ�����ʾ����Ļ��, ������Ϣԭ������;

