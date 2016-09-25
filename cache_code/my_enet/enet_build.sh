#!/bin/sh

enet_output=`pwd`/enet2
echo $enet_output

cd enet-1.3.13
make clean
./configure --prefix=$enet_output  --disable-shared --enable-static
make & make install

