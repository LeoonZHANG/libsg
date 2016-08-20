# ![](logo.png)

## Overview

libsg (library sigma) is a boost-like cross-platform C utility library focusing on usability.

## Idea and goal
C language is gradually being forgotten, how to make C programing easier?

1, Do not reinvent wheels, pack good wheels.

2, Using C to build important infrastructure, not everything.

## Really easy

#### Easy to install

You can download pre-built binaries (include static, dynamic library and header files) which doesn't have any dependencies, you don't need to install any dependencies by yourself.

#### Easy to call

Header files of libsg include nothing but only pure API declaration, and they are simpe and clear.

## Todo list
Add CMake script.

Add http / tcp / ring_buffer / blocking_queue modules.

Release version 0.1.

...

## Download releases

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

## Supported Platforms

Linux ／ Windows ／ OS X

## Build Instructions

#### Linux

#### Windows

#### OS X

1. brew is recommended for installing the dependencies, please visit http://brew.sh for setup brew.

2. Use brew to install dependencies, take libuv as an instance:

        $ brew install libuv

3. Run follow command in project directory to generate XCode project:

        $ cmake -G'Xcode'

4. open the generated project file `libsg.xcodeproj` by XCode, build and enjoy it.

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
