#!/bin/sh
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    brew update
    brew install xmake
else
    curl -o xmake.tar.gz https://codeload.github.com/waruqi/xmake/tar.gz/v2.0.4
    tar xf xmake.tar.gz
    cd xmake-2.0.4 && sudo ./install
fi
