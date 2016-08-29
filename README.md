# ![](res/image/logo.png)

[![Build Status](https://travis-ci.org/metalwood/libsg.svg?branch=master)](https://travis-ci.org/metalwood/libsg)

## Overview

Cross-platform C utility library focusing on efficient development and easy-to-use.

## Idea

1, Reuse, not rework or reinvent.

2, Using C to build important infrastructure, not everything.

## Feature

#### Efficient development

Common C API vs libsg API sample

#### Easy-to-install source code

Official pre-built binaries don't have any dependency, you don't need to build or install dependency for libsg.

#### Easy-to-understand APIs

Header files of libsg include nothing but only pure API declaration, and APIs will not cause misunderstanding.

Common C API vs libsg API sample

#### Performance

Key modules are optimized, you can create high performance application with libsg.



## Supported Platforms

Linux ／ Windows ／ OS X

## Download > Install > Use


## Modules

####sys
atom

mutex

rwlock

spin_lock

memory

module

os

proc

time

file

flag

shell

sleep

thread

ntp

####str

charset

regex

json

sprintf

vlstr(variable length string)

xml

msgpack

####net

tcp/tcp_server: async tcp client and server based on LIBUV.

udp/udp_server: async udp client and server based on LIBUV.

etp/etp_server: a extreme transfer protocol based on KCP / LIBUV(UDP) and FEC, it's fast and reliable.

http/http_server: http client and server based on libcurl and libwebsocket.

websocket: websocket client based on libwebsocket.

ftp: ftp client based on libcurl.

####db

mysql

mongodb

redis

sqlite

postgresql

####container

ring_buf

list

queue

blocking_queue

####compress

zip

gzip

####crypto

md: message digest algorithm pack including MD2/MD4/MD5/SHA1/SHA224/SHA256/SHA512.

mac: message authentication code algorithm pack including HmacMD5...

sym: symmetric encryption algorithm pack including DES/3DES/AES.

####hash

uuid

murmur

####media

capture_v

capture_a

####math

c_int

c_float

big_int

big_float

####platform

windows/dial...

####util

assert

dump

log

## Build Instructions

If you want to use libsg in your project, just download the lastest release. Build libsg when you are going to change the code.

        mkdir build
        cd build
        cmake ..
        make -j4


## API document

## Demo

## Code specification

English: [Linux kernel coding style](https://www.kernel.org/doc/Documentation/CodingStyle)

简体中文: [Linux内核编码风格](http://www.cnblogs.com/baochuan/archive/2013/04/08/3006615.html)

Tool: http://astyle.sourceforge.net/astyle.html --style=linux

## Contact

email: metalwood@foxmail.com

QQ group: 556925561

## Contributors
metalwood (metalwood@foxmail.com)

cole_yang (2973778938@qq.com)

canmor (476010464@qq.com)

...

