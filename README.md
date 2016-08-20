# ![](logo.png)

## Overview

libsg is a boost-like cross-platform C utility library, it's high-performance and easy to use.

## Idea

C language is gradually being forgotten, how to make C programing easier?

1, Do not reinvent wheels, pack good wheels.

2, Using C to build important infrastructure, not everything.

## High-performance

Lots of modules in libsg are optimized, you can create high performance application with libsg.

## Easy to use

#### Easy to install

You can [download pre-built binaries](https://github.com/metalwood/libsg/releases) (static / dynamic library and header files) which don't have any dependency, you don't need to build source code or install any dependency for libsg.

#### Easy to call

Header files of libsg include nothing but only pure API declaration, and APIs are simpe and clear.

## Supported Platforms

Linux ／ Windows ／ OS X

## [Download releases](https://github.com/metalwood/libsg/releases)

## Modules

####sys
atom, mutex, rwlock, spin_lock, memory, module, os, proc, time, file, flag, shell, sleep, thread, ntp

####str
charset, regex, json, sprintf, vlstr(variable length string), xml, msgpack, bson

####net

tcp/tcp_server: async tcp client and server based on LIBUV.

udp/udp_server: async udp client and server based on LIBUV.

etp/etp_server: a extreme transfer protocol based on KCP / LIBUV(UDP) and FEC, it's fast and reliable.

http/http_server: http client and server based on libcurl and libwebsocket.

websocket: websocket client based on libwebsocket.

ftp: ftp client based on libcurl.

####db
mysql, mongodb, redis, sqlite, postgresql,unqlite...

####container
ring_buf, list, queue, blocking_queue...

####compress
zip, gzip, md5, sha1...

####crypt
...

####media
capture_v, capture_a, 

####math
number, uuid

####platform
windows/dial...

####util
assert, dump, log...

## Build Instructions

If you want to use libsg in your project, just download the lastest release. Build libsg when you are going to change the code.

#### Linux
        $ chmod +x build_linux.sh & ./build.sh

#### OSX
        $ chmod +x build_linux.sh & ./build.sh
open the generated project file `libsg.xcodeproj` by XCode, build it.

#### Windows
        $ build.bat
open the generated project file `libsg.sln` by VisualStudio, build it.

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

## Todo list
Add CMake script.

Add http / tcp / ring_buffer / blocking_queue modules.

Release version 0.1.

...
