## Todo list
1100
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

1500

第一步: 制定需求
对模块如何实现,基于哪个开源项目,在不同平台对实现,易用易懂的接口等, 进行梳理,编写可靠的接口,形成模块的头文件。易用易懂看似简单,但是对接口设计影响很大, 也非常重要,请参照md等模块在这方面对体现。

第二步: 实现
按需求实现头文件, 进行单元测试, 对部分必要对模块进行性能测试

模块如下:
msgpack msgpack操作库,要求满足常用对读写需要,可以参考json模块,当然,json有json pointer,包括msgpack和json相互转换,其中,对_b64后缀结束的json字段自动进行base64的转换(官方的转换工具中貌似有什么base64识别规则)
pigz 同上。https://github.com/kjk/pigz (unix-like) 加上 https://github.com/madler/pigz (windows port)
fs file+dir的常用接口 file的常用操作基本都有了,在原来的file模块中,dir的实现在libsg中也有一部分, 不太满意, 但是dir的跨平台要多花点时间。搞定之后删除file模块。
rwlock 跨平台高性能rwlock封装,一个h文件,多个c文件,如下。如果需要,可以调整c文件的分布结构。
      rwlock_atom.c    基于https://github.com/malbrain/rwlock/blob/master/readerwriter.c封装或者实现,可以#include这个c文件,这样尽量不修改其源码
      rwlock_api.c 系统读写锁系统API的封装,linux的pthread,windows7以及以上也有, 
      rwlock_winxp.c   winxp需要其他方法模拟,网上大把,找个简单的即可,性能无所谓
      除了单元测试,还要做atom与系统api两种实现之间的对比
spin_lock 同上,要求跨平台, 注意h文件的接口要一目了然
list 基于现有代码重构, 删掉不该list包括对检索、开辟内存等等数据结构不该管的,就可以了。
queue(支持非阻塞和阻塞) 基于开源代码重构,没必要引用
kfifo: 啥都没写, 从内核代码里扒, 按libsg一贯的接口设计原则和风格重构一下,如果有
sqlite 简化官方接口
BerkeleyDB 简化官方接口
ejdb 简化官方接口
murmur 简化官方接口, 官方接口的输入和输出让人费解, 不够一目了然,比如,对一个很大的文件怎样计算murmur哈希,比如murmur哈希值到底多个字节,怎样存放


------------------------------------------------------------

http_server 重构http服务器, h2o + libwebsocket
zip重构 zlib zip gzip

－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－ Release

kcp + reed-solomon
Nat traversal, P2P

－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－
//非对称加密算法 asym: RSA、Elgamal、背包算法、Rabin、D-H、ECC
//数字签名算法
//数字证书
bitmap
  hash_set
  hash_map
  tree_map
  tree_set
str_hash 字符哈希方法集合
c－blosc
7z
snappy 快速数据压缩解压,直接对文件、字符串喝buffer进行操作对接口要有, 累计填充数据对偏底层接口也要有。

dial 增加其他平台支持? 编程调用bat也可以了其实
  
speed_test 速度测试

hls hls客户端？需要这个模块吗？

rtmp rtmp客户端，和rtsp的操作方法基本一致

rtmp_server rtmp服务器

rtsp_server rtsp服务器

hls_server HLS服务器

raft 分布式raft封装？

tor tor代理

socks_proxy_server

p2p 跨局域网点对点通信

media系列的