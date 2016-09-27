##apr

url: https://github.com/apache/apr/releases

version: 1.5.2

##FastMemcpy

url: https://github.com/skywind3000/FastMemcpy

version: commit a74a33a

##jemalloc

url: https://github.com/jemalloc/jemalloc

version: 4.2.1

##dablooms

url: https://github.com/bitly/dablooms

version: commit cac1c09

##tbox

url: https://github.com/waruqi/tbox

version: 1.5.4 (not release, commit 5c8b19f, 2016-9-27)

##pigz

url: 

version: 2.3.3

##pigs-win

url: https://github.com/kjk/pigz/

version: 2.3

##openssl

url: https://github.com/openssl/openssl

version: 1.1.0

##mpir

url: http://mpir.org

version: 2.7.2

##zlib

url: http://www.zlib.net

version: 1.2.8

modify:
     - 016aa9e8142e580f381f67d66f49c7eaeb41044f Fix issue in zlib/CMakeLists.txt 为了支持不在源目录cmake，比如：cd build & cmake ..
        
##libuuid

url: linux repo

version: unknown
    
modify:
    - 27c1921c10bf10cd53b1c5721708a97bfba7b191 Lower autoconf version required from libuuid 为了支持Travis-CI，降低autoconf的版本要求
    - 270a61f710421ffd3cf76ab93039deb599903e15 Add precompiled header 添加了预编译的头文件，才能编译通过
        
##pcre

url: www.pcre.org

version: unknown

modify:
    - 270a61f710421ffd3cf76ab93039deb599903e15 Fix compile issues on Linux without dependencies 修复某些版本cmake会报错
    - fdd730399c7187be9f2a2643520a7024a574a789 Fix syntax error in pcre/CMakeLists.txt 修复cmake语法问题
        
##libiconv (for windows)

url: 

version:

modify:
    - bb794977e10269bf87d750a3dcf4da69c7035898 为Windows添加CMakeFiles.txt
        
##libiconv-1.14 (for posix)

url:

version:

modify:
    - 某些版本的linux下需要更改一行代码才能编译通过，但是没有直接改代码，而是写在3rdparty/CMakeLists.txt里用命令改。

##c99-snprintf

url:

version:

modify:
    - 1a1950c23d4efb870ee5e49923feebb3f35b3ca9 为Windows添加CMakeFiles.txt