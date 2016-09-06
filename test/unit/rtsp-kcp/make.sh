gcc camera.c ../../../src/net/etp.c ../../../src/net/etp_server.c ../../..//src/media/rtsp.c ../../../3rdparty/kcp/ikcp.c ../../../3rdparty/linkhash/linkhash.c  -I../../../3rdparty/kcp/ -I../../../3rdparty/linkhash/ -I../../../include  -lpthread -lcurl -luv -o camera

gcc watch.c ../../../src/net/etp.c ../../..//src/media/player.c ../../../3rdparty/kcp/ikcp.c  -I../../../3rdparty/kcp/ -I../../../include  -lpthread -lcurl -luv -lvlc -o watch

ffmpeg -y -i rtsp://admin:12345@192.168.5.10 -vcodec copy -acodec copy -f mpegts udp://127.0.0.1:8071?pkt_size=1316