# ![](logo.png)

## Overview

libsg (library sigma) is a cross-platform c library focused on usability.

## Idea
C language is gradually being forgotten, how to make C programing easier?

1, Do not reinvent wheels, encapsulate good wheels.

2, Using C to build important infrastructure, not everything.

## Download releases

## Modules

####sys
atom, mutex, rwlock, spin_lock, memory, module, os, proc, time, file, flag, shell, sleep, thread, ntp

####char
charset, regex, json, sprintf, vlstr(variable length string), xml, msgpack, bson

####net
tcp, tcp_server, udp, udp_server, rudp(reliable udp), rudp_server, http, http_server, websocket, ftp, email, rtmp, ip...

####db
mysql, mongodb, redis, sqlite, postgresql,unqlite...

####container
ring_buf, list, queue, blocking_queue...

####compress(hash)
zip, gzip, md5...

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

Linux using the GCC toolchain.

Windows using the GCC or msvc toolchain.

OS X using the GCC or XCode toolchain.

## Build Instructions

#### Linux

#### Windows

#### OS X

## Documents

## Dependencies

libuv,...
