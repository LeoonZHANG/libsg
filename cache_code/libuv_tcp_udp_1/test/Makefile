CC=gcc
#修改路径为libuv的路径
INCLUDES = -I/Users/sundq/workspace/libuv/include
LIBS = -L/Users/sundq/workspace/libuv/build/Release/

CCFLAGS = -g
UVZOBJS = obj/tcp.o obj/tcp_server.o obj/udp.o obj/udp_server.o

all: obj bin bin/tcp_udp_server bin/udp_client bin/tcp_client bin/performance_test

obj:	
	mkdir -p obj
bin:
	mkdir -p bin
bin/tcp_udp_server: obj/tcp_udp_server.o ${UVZOBJS}
	$(CC) obj/tcp_udp_server.o ${UVZOBJS} $(CCFLAGS) -o $@ $(INCLUDES) $(LIBS) -luv
bin/udp_client: obj/udp_client.o ${UVZOBJS}
	$(CC) obj/udp_client.o ${UVZOBJS} $(CCFLAGS) -o $@ $(INCLUDES) $(LIBS) -luv
bin/tcp_client: obj/tcp_client.o ${UVZOBJS}
	$(CC) obj/tcp_client.o ${UVZOBJS} $(CCFLAGS) -o  $@ $(INCLUDES) $(LIBS) -luv
bin/performance_test: obj/performance_test.o ${UVZOBJS}
	$(CC) obj/performance_test.o $(CCFLAGS) ${UVZOBJS} -o  $@ $(INCLUDES) $(LIBS) -luv
obj/tcp_udp_server.o: tcp_udp_server.c	
	${CC} -c tcp_udp_server.c $(CCFLAGS) -o $@ $(INCLUDES)	
obj/udp_client.o: udp_client.c	
	${CC} -c udp_client.c $(CCFLAGS) -o $@ $(INCLUDES)
obj/tcp_client.o: tcp_client.c	
	${CC} -c tcp_client.c $(CCFLAGS) -o $@ $(INCLUDES)		
obj/tcp.o: ../tcp.c
	$(CC) -c ../tcp.c $(CCFLAGS) -o $@ $(INCLUDES)
obj/udp.o: ../udp.c
	$(CC) -c ../udp.c $(CCFLAGS) -o $@ $(INCLUDES)	
obj/tcp_server.o: ../tcp_server.c 
	$(CC) -c ../tcp_server.c $(CCFLAGS) -o $@ $(INCLUDES)
obj/udp_server.o: ../udp_server.c
	$(CC) -c ../udp_server.c $(CCFLAGS) -o $@ $(INCLUDES)
obj/performance_test.o: performance_test.c	
	${CC} -c performance_test.c $(CCFLAGS) -o $@ $(INCLUDES)	


clean:
	rm -rf bin
	rm -rf obj
