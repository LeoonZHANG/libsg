#! /bin/bash
# 运行环境：debian 8
# 最终生成的文件在脚本同一层的ffmpeg_result目录

# 任何命令出错，自动退出脚本
set -e

# 用 "PROXY=代理 ./build-ffmpeg-win.sh" 的形式指定代理，如果未指定则不使用代理
http_proxy=$PROXY
ftp_proxy=$PROXY

apt-get install -y build-essential curl mingw-w64 pkg-config yasm

# 创建压缩包存放目录
mkdir -p tarball

# 下载ffmpeg源码包
ffmpeg_version='3.1.3'
ffmpeg_url="http://ffmpeg.org/releases/ffmpeg-${ffmpeg_version}.tar.xz"
ffmpeg_filename=`basename ${ffmpeg_url}`
if [ ! -e "tarball/${ffmpeg_filename}" ];then
  curl -# -fsSL ${ffmpeg_url} > tarball/${ffmpeg_filename}
fi

# 清空所有文件，避免残留文件影响
rm -rf src
mkdir -p src
pushd src

# 使用heredoc，方便扩展成多行
cat << EOF
Extracting source files,keep relax.^_^
EOF

# 解压源码包
tar -xvf ../tarball/${ffmpeg_filename}

# 使用heredoc，方便扩展成多行
cat << EOF
Time to modify source code in `pwd`.
Press any key to continue
EOF
# 按回车键，继续执行
read

ffmpeg_src=${ffmpeg_filename%.*.*}
pushd ${ffmpeg_src}
./configure --prefix=$(pwd)/../../ffmpeg_result --enable-shared --arch=x86_64 --target-os=mingw32 --cross-prefix=x86_64-w64-mingw32- --pkg-config=pkg-config
make
make install
make clean
./configure --prefix=$(pwd)/../../ffmpeg_result_i686 --enable-shared --arch=i686 --target-os=mingw32 --cross-prefix=i686-w64-mingw32- --pkg-config=pkg-config
make
make install
popd

popd
