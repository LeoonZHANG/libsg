#bin/sh
echo "清除文件"
rm -Rf bin/*

echo "编译server_test"
gcc -std=c99 -O0 -g3 -Wall -fmessage-length=0 -fexec-charset=utf-8 -o bin/server_test server/*.c -Ienet/include  -Lenet/lib -lenet 

echo "编译client_test"
gcc -std=c99 -O0 -g3 -Wall -fmessage-length=0 -fexec-charset=utf-8 -o bin/client_test client/*.c -Ienet/include  -Lenet/lib -lenet
