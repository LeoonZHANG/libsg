#! /bin/bash
# 最终生成的文件在脚本同一层的ffmpeg_result目录

# 任何命令出错，自动退出脚本
set -e

# 用 "PROXY=代理 ./build-ffmpeg-macos.sh" 的形式指定代理，如果未指定则不使用代理
http_proxy=$PROXY
ftp_proxy=$PROXY

# 创建压缩包存放目录
mkdir -p tarball

# 下载ffmpeg源码包
ffmpeg_version='3.1.3'
ffmpeg_url="http://ffmpeg.org/releases/ffmpeg-${ffmpeg_version}.tar.xz"
ffmpeg_filename=`basename ${ffmpeg_url}`
if [ ! -e "tarball/${ffmpeg_filename}" ];then
  curl -# -fsSL ${ffmpeg_url} > tarball/${ffmpeg_filename}
fi

# 下载pkg-config
pkg_config_version='0.29.1'
pkg_config_url="https://pkg-config.freedesktop.org/releases/pkg-config-${pkg_config_version}.tar.gz"
pkg_config_filename=`basename ${pkg_config_url}`
if [ ! -e "tarball/${pkg_config_filename}" ];then
  curl -# -fsSL ${pkg_config_url} > tarball/${pkg_config_filename}
fi

# 下载yasm
yasm_version='1.3.0'
yasm_url="http://www.tortall.net/projects/yasm/releases/yasm-${yasm_version}.tar.gz"
yasm_filename=`basename ${yasm_url}`
if [ ! -e "tarball/${yasm_filename}" ];then
  curl -# -fsSL ${yasm_url} > tarball/${yasm_filename}
fi

# 清空所有文件，避免残留文件影响
rm -rf src
mkdir -p src/fake
pushd src
export PATH=$(cd fake;pwd)/bin:$PATH

# 使用heredoc，方便扩展成多行
cat << EOF
Extracting source files,keep relax.^_^
EOF

# 解压源码包
tar -xvf ../tarball/${pkg_config_filename}
tar -xvf ../tarball/${yasm_filename}
tar -xvf ../tarball/${ffmpeg_filename}

# 使用heredoc，方便扩展成多行
cat << EOF
Time to modify source code in `pwd`.
Press any key to continue
EOF
# 按任意键，继续执行
read

pkg_config_src=${pkg_config_filename%.*.*}
pushd ${pkg_config_src}
./configure --prefix=$(cd ../fake;pwd) --with-internal-glib
make
make install
popd

yasm_src=${yasm_filename%.*.*}
pushd ${yasm_src}
PKG_CONFIG_PATH=$(cd ../fake;pwd)/lib/pkgconfig ./configure --prefix=$(cd ../fake;pwd)
make
make install
popd

ffmpeg_src=${ffmpeg_filename%.*.*}
pushd ${ffmpeg_src}
PKG_CONFIG_PATH=$(cd ../fake;pwd)/lib/pkgconfig ./configure --prefix=$(cd ../..;pwd)/ffmpeg_result --enable-shared
make
make install
popd

popd
