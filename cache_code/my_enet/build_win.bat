echo "清除文件"
mkdir bin
del -Rf bin/*

echo "编译server_test"
gcc -std=c99 -O0 -g3 -Wall -fmessage-length=0 -fexec-charset=utf-8 -o bin/server_test server/*.c -Ienet_win/include  -Lenet_win/lib  -lenet64 -lws2_32 -lmswsock -lwinmm 

echo "编译client_test"
gcc -std=c99 -O0 -g3 -Wall -fmessage-length=0 -fexec-charset=utf-8 -o bin/client_test client/*.c -Ienet_win/include  -Lenet_win/lib  -lenet64 -lws2_32 -lmswsock -lwinmm 
