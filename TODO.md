## Todo list

big_int 基于mpir的大整数模块 80

big_float 基于mpir的大浮点数模块 80

net_card 调用系统API,获取网卡基本信息 100

random “真”随机数生成,可以基于第三方库 50

speed 速度换算和计数器 100

bloom_filter 基于开源布隆过滤器bloomd封装的布隆过滤器,接口基本完全一致 100 

digest_hash 基于openssl的接口封装的常见摘要算法,包括md5、sha1、sha256、sha512

reg_crypto 基于openssl的接口封装的常见加密算法, 包括aes、ces、rabbit、rc4、tripleDes




crc32

msgpack msgpack操作库

msgpack_json_conv msgpack和json相互转换

snappy 快速数据压缩解压 c－blocs

pig-z ?

7z

kcp + reed-solomon

kfifo

dl_list 重构

queue(支持非阻塞和阻塞)

bitmap hash_set hash_map、tree_map、tree_set 用途？

dir重构

dial 增加其他平台支持

rwlock

spin_lock

str_hash 字符哈希方法集合

－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－

speed_test 速度测试

hls hls客户端？需要这个模块吗？

rtmp rtmp客户端，和rtsp的操作方法基本一致

rtmp_server rtmp服务器

rtsp_server rtsp服务器

hls_server HLS服务器

raft 分布式raft封装？

http_server 重构http服务器

tor tor代理

socks_proxy_server

p2p 跨局域网点对点通信

media系列的