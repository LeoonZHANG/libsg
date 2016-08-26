## Todo list
big_int: 基于mpir的大整数模块
big_float: 基于mpir的大浮点数模块
net_card: 调用系统API,获取网卡基本信息
random: “真”随机数生成, 可以基于tbox做
speed: 速度换算和计数器
bloom_filter: 基于开源布隆过滤器bloomd封装的布隆过滤器, 接口基本完全一致, 头文件已经写了开源库的地址
md: 基于openssl到对常用消息摘要算法的封装
mac: 基于openssl到对常用消息认证码算法的封装
sym: 基于openssl到对常用对称加密算法的封装
crc: crc8/ccitt/16/32/64, github等地方代码大把, 封装到一起, 可以基于tbox的实现做, crc64的另找开源实现


msgpack_json_conv: msgpack和json相互转换,其中,对_b64后缀结束的json字段自动进行base64的转换
msgpack msgpack操作库
snappy 快速数据压缩解压 c－blocs
pig-z ?
7z
dl_list 重构
queue(支持非阻塞和阻塞)
kfifo: 啥都没写, 从内核代码里扒, 按libsg一贯的接口设计原则和风格重构一下
bitmap / hash_set / hash_map / tree_map / tree_set
dir重构
dial 增加其他平台支持
rwlock
spin_lock
str_hash 字符哈希方法集合

kcp + reed-solomon

－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－
//非对称加密算法 asym: RSA、Elgamal、背包算法、Rabin、D-H、ECC
//数字签名算法
//数字证书

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