# ![](logo.png)

## Overview

libsg (library sigma) is a cross-platform C utility library focusing on usability.

## Idea
C language is gradually being forgotten, how to make C programing easier?

1, Do not reinvent wheels, encapsulate good wheels.

2, Using C to build important infrastructure, not everything.

## Todo list
Add CMake script.

Add http server / kcp / tcp modules.

Release version 0.1.

...

## Download releases

## Modules

####sys
atom, mutex, rwlock, spin_lock, memory, module, os, proc, time, file, flag, shell, sleep, thread, ntp

####str
charset, regex, json, sprintf, vlstr(variable length string), xml, msgpack, bson

####net
tcp, tcp_server, udp, udp_server, rudp(reliable udp), rudp_server, http, http_server, kcp, kcp_server, websocket, ftp, email, rtmp, ip...

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

Linux kernel style.

## Contact

email: metalwood@foxmail.com

QQ group: 556925561

## Contributors
metalwood (metalwood@foxmail.com)

cole_yang (2973778938@qq.com)

canmor (476010464@qq.com)

...
