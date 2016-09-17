#!/usr/bin/env bash
# install lua 5.3.3 binary and dev files

curl -R -O http://www.lua.org/ftp/lua-5.3.3.tar.gz
tar zxf lua-5.3.3.tar.gz
cd lua-5.3.3

if [ "$(uname)" == "Darwin" ]; then
    make macosx test
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    make linux test
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
    # Do something under Windows NT platform
fi

sudo make install