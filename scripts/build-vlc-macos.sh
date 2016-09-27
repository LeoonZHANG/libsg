#! /bin/bash
# 最终生成的文件在脚本同一层的vlc_result目录

# 任何命令出错，自动退出脚本
set -e

# 用 "PROXY=代理 ./vlc.sh" 的形式指定代理，如果未指定则不使用代理
http_proxy=$PROXY
ftp_proxy=$PROXY

# 创建压缩包存放目录
mkdir -p tarball

# 下载vlc源码包
vlc_version='2.2.4'
vlc_url="http://get.videolan.org/vlc/2.2.4/vlc-${vlc_version}.tar.xz"
vlc_filename=`basename ${vlc_url}`
if [ ! -e "tarball/${vlc_filename}" ];then
  curl -# -fsSL ${vlc_url} > tarball/${vlc_filename}
fi

# 下载xz源码包
xz_version='5.2.2'
xz_url="http://tukaani.org/xz/xz-${xz_version}.tar.gz"
xz_filename=`basename ${xz_url}`
if [ ! -e "tarball/${xz_filename}" ];then
  curl -# -fsSL ${xz_url} > tarball/${xz_filename}
fi

# 下载libxml2源码包
libxml2_version='2.9.4'
libxml2_url="ftp://xmlsoft.org/libxml2/libxml2-${libxml2_version}.tar.gz"
libxml2_filename=`basename ${libxml2_url}`
if [ ! -e "tarball/${libxml2_filename}" ];then
  curl -# -fsSL ${libxml2_url} > tarball/${libxml2_filename}
fi


# 清空所有文件，避免残留文件影响
rm -rf src
mkdir -p src/fake
pushd src

# 使用heredoc，方便扩展成多行
cat << EOF
正在解压源码，可以放松一下^_^
EOF

# 解压源码包
tar -xvf ../tarball/${xz_filename}
tar -xvf ../tarball/${libxml2_filename}
tar -xvf ../tarball/${vlc_filename}

# 使用heredoc，方便扩展成多行
cat << EOF
解压源码完成，如需修改请现在就做，源码路径：`pwd`。
修改完毕按任意键继续……
EOF
# 按任意键，继续执行
read

xz_src=${xz_filename%.*.*}
pushd ${xz_src}
# 参数需要指定绝对路径
./configure --prefix=$(cd ../fake;pwd)
make
make install
popd


libxml2_src=${libxml2_filename%.*.*}
pushd ${libxml2_src}
./configure --prefix=$(cd ../fake;pwd) --with-lzma=$(cd ../fake;pwd) --with-python-install-dir=$(cd ../fake;pwd)
make
make install
popd

tar -xvf ../tarball/${vlc_filename}
vlc_src=${vlc_filename%.*.*}
mkdir ${vlc_src}/build
pushd ${vlc_src}/build
# 暂时设置，语句出错不退出
# 设置环境变量
export CC="xcrun clang"
export CXX="xcrun clang++"
export OBJC="xcrun clang"
set +e
# 执行编译脚本
../extras/package/macosx/build.sh
# 恢复语句出错处理的设置
set -e
# 获取要替换的文件路径
libpath=$(dirname $(find .. -name libxml2.a))
cp ../../fake/lib/libxml2.{,l}a $libpath
# 再次执行编译脚本
../extras/package/macosx/build.sh
cd ..
cp -r build ../../vlc_result
popd

popd
