#! /bin/bash
# 运行环境：debian8 ubuntu16 ubuntu14 centos7 centos6
# 最终生成的文件在脚本同一层的vlc_result目录

# 任何命令出错，自动退出脚本
set -e

# 用 "PROXY=代理 ./build-vlc-win.sh" 的形式指定代理，如果未指定则不使用代理
http_proxy=$PROXY
ftp_proxy=$PROXY

if [ $# -lt 1 ];then
cat << EOF
Usage: ./build-vlc-linux.sh dist [version]
specify version to build git version vlc
when tarball/vlc folder exists, specify version as latest will pull latest code. Other value will use exist vlc
Note: change on vlc code may require change of dependencies's version, or even build steps
possible dist can be one of debian,ubuntu16,ubuntu14,centos7,centos6
EOF
  exit 1
else
  dist=$1
fi

if [ $# -ge 2 ];then
  # centos6的系统库不支持C++11，而git版本的vlc有部分代码是必须要支持C++11标准才能编译的。
  # 所以git版vlc不支持在centos6编译
  if [ ${dist} == 'centos6' ];then
    echo "git version of vlc can't be built on centos6"
    exit 1
  fi
  vlc_version=$2
fi

case ${dist} in
debian|ubuntu16|ubuntu14)
  apt-get install -y build-essential curl xz-utils pkg-config autoconf libtool git
  ;;
centos7|centos6)
  yum install -y gcc-c++ make xz pkgconfig python-devel autoconf automake libtool git
  ;;
*)
cat << EOF
Usage: ./build-vlc-linux.sh dist [version]
specify version to build git version vlc
when tarball/vlc folder exists, specify version as latest will pull latest code. Other value will use exist vlc
Note: change on vlc code may require change of dependencies's version, or even build steps
possible dist can be one of debian,ubuntu16,ubuntu14,centos7,centos6
EOF
  exit 1
  ;;
esac

# 创建压缩包存放目录
mkdir -p tarball

# 下载vlc源码包
if [ "x${vlc_version}" != 'x' ];then
  if [ ! -d tarball/vlc ];then
    git clone git://github.com/videolan/vlc tarball/vlc
  else
    if [ ${vlc_version} == 'latest' ];then
      pushd tarball/vlc && git checkout -f master && git pull && popd
    fi
  fi
else
  vlc_version='2.2.4'
  vlc_url="http://get.videolan.org/vlc/${vlc_version}/vlc-${vlc_version}.tar.xz"
  vlc_filename=`basename ${vlc_url}`
  if [ ! -e "tarball/${vlc_filename}" ];then
    curl -# -fsSL ${vlc_url} > tarball/${vlc_filename}
  fi
fi

# 下载yasm源码包
yasm_version=1.3.0
yasm_url="http://www.tortall.net/projects/yasm/releases/yasm-${yasm_version}.tar.gz"
yasm_filename=`basename ${yasm_url}`
if [ ! -e "tarball/${yasm_filename}" ];then
  curl -# -fsSL ${yasm_url} > tarball/${yasm_filename}
fi

# 下载libmad源码包
libmad_version='0.15.1b'
libmad_url="ftp://ftp.mars.org/pub/mpeg/libmad-${libmad_version}.tar.gz"
libmad_filename=`basename ${libmad_url}`
if [ ! -e "tarball/${libmad_filename}" ];then
  curl -# -fsSL ${libmad_url} > tarball/${libmad_filename}
fi

# 下载libav源码包
if [ "x${vlc_filename}" == 'x' ];then
  if [ ! -d tarball/libav ];then
    git clone git://github.com/libav/libav.git tarball/libav
  else
    if [ ${vlc_version} == 'latest' ];then
      pushd tarball/libav && git checkout -f master && git pull && popd
    fi
  fi
else
  libav_version='11.8'
  libav_url="https://libav.org/releases/libav-${libav_version}.tar.gz"
  libav_filename=`basename ${libav_url}`
  if [ ! -e "tarball/${libav_filename}" ];then
    curl -# -fsSL ${libav_url} > tarball/${libav_filename}
  fi
fi

# 下载liba52源码包
liba52_version='0.7.4'
liba52_url="http://liba52.sourceforge.net/files/a52dec-${liba52_version}.tar.gz"
liba52_filename=`basename ${liba52_url}`
if [ ! -e "tarball/${liba52_filename}" ];then
  curl -# -fsSL ${liba52_url} > tarball/${liba52_filename}
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

# 下载gawk源码包
gawk_version='4.1.4'
gawk_url="http://mirrors.ustc.edu.cn/gnu/gawk/gawk-${gawk_version}.tar.xz"
gawk_filename=`basename ${gawk_url}`
if [ ! -e "tarball/${gawk_filename}" ];then
  curl -# -fsSL ${gawk_url} > tarball/${gawk_filename}
fi

# 下载ncurses源码包
ncurses_version='6.0'
ncurses_url="http://ftp.gnu.org/gnu/ncurses/ncurses-${ncurses_version}.tar.gz"
ncurses_filename=`basename ${ncurses_url}`
if [ ! -e "tarball/${ncurses_filename}" ];then
  curl -# -fsSL ${ncurses_url} > tarball/${ncurses_filename}
fi

# 下载readline源码包
readline_version='7.0'
readline_url="ftp://ftp.gnu.org/gnu/readline/readline-${readline_version}.tar.gz"
readline_filename=`basename ${readline_url}`
if [ ! -e "tarball/${readline_filename}" ];then
  curl -# -fsSL ${readline_url} > tarball/${readline_filename}
fi

# 下载lua源码包
lua_version='5.2.4'
lua_url="http://www.lua.org/ftp/lua-${lua_version}.tar.gz"
lua_filename=`basename ${lua_url}`
if [ ! -e "tarball/${lua_filename}" ];then
  curl -# -fsSL ${lua_url} > tarball/${lua_filename}
fi

# 下载gettext源码包
gettext_version='0.19.8.1'
gettext_url="http://ftp.gnu.org/gnu/gettext/gettext-${gettext_version}.tar.gz"
gettext_filename=`basename ${gettext_url}`
if [ ! -e "tarball/${gettext_filename}" ];then
  curl -# -fsSL ${gettext_url} > tarball/${gettext_filename}
fi

# 清空所有文件，避免残留文件影响
rm -rf src
mkdir -p src/fake
pushd src
# 设置PATH变量，自动使用编译的工具
export PATH=$(cd fake;pwd)/bin:$PATH
# 设置LD_LIBRARY变量，自动使用编译的库
export LD_LIBRARY_PATH=$(cd fake;pwd)/lib

# 使用heredoc，方便扩展成多行
cat << EOF
Extracting source files,keep relax.^_^
EOF

# 解压源码包
if [ "x${vlc_filename}" == 'x' ];then
  cp -r ../tarball/vlc .
else
  tar -xvf ../tarball/${vlc_filename}
fi
tar -xvf ../tarball/${yasm_filename}
tar -xvf ../tarball/${libmad_filename}
if [ "x${libav_filename}" == 'x' ];then
  cp -r ../tarball/libav .
else
  tar -xvf ../tarball/${libav_filename}
fi
tar -xvf ../tarball/${liba52_filename}
tar -xvf ../tarball/${xz_filename}
tar -xvf ../tarball/${libxml2_filename}
tar -xvf ../tarball/${gawk_filename}
tar -xvf ../tarball/${ncurses_filename}
tar -xvf ../tarball/${readline_filename}
tar -xvf ../tarball/${lua_filename}
tar -xvf ../tarball/${gettext_filename}

# 使用heredoc，方便扩展成多行
cat << EOF
Time to modify source code in `pwd`.
Press any key to continue
EOF
# 按回车键，继续执行
read

# 因为有些发行版自带的yasm版本不够新，所以在脚本里编译最新版
yasm_src=${yasm_filename%.*.*}
pushd ${yasm_src}
./configure --prefix=$(cd ../fake;pwd)
make
make install
popd

libmad_src=${libmad_filename%.*.*}
pushd ${libmad_src}
sed -i '/-fforce/d' configure
./configure --prefix=$(cd ../fake;pwd)
make
make install
popd

if [ "x${libav_filename}" == 'x' ];then
  libav_src=libav
else
  libav_src=${libav_filename%.*.*}
fi
pushd ${libav_src}
./configure --prefix=$(cd ../fake;pwd) --enable-shared
make
make install
popd

liba52_src=${liba52_filename%.*.*}
pushd ${liba52_src}
CFLAGS='-fPIC' ./configure --prefix=$(cd ../fake;pwd)
make
make install
popd

xz_src=${xz_filename%.*.*}
pushd ${xz_src}
./configure --prefix=$(cd ../fake;pwd)
make
make install
popd

libxml2_src=${libxml2_filename%.*.*}
pushd ${libxml2_src}
./configure --prefix=$(cd ../fake;pwd) --with-lzma=$(cd ../fake;pwd) --without-python
make
make install
popd

# 部分发行版需要用gawk代替mawk才能正常编译ncurses，为规避版本不一致问题自己编译一份
gawk_src=${gawk_filename%.*.*}
pushd ${gawk_src}
./configure --prefix=$(cd ../fake;pwd)
make
make install
popd

# 修改编译脚本，去掉mawk的可选项
ncurses_src=${ncurses_filename%.*.*}
pushd ${ncurses_src}
sed -i s/mawk// configure
CFLAGS='-fPIC' ./configure --prefix=$(cd ../fake;pwd)
make
make install
popd

readline_src=${readline_filename%.*.*}
pushd ${readline_src}
LDFLAGS="-L$(cd ../fake;pwd)/lib" ./configure --prefix=$(cd ../fake;pwd)
make
make install
popd

lua_src=${lua_filename%.*.*}
pushd ${lua_src}
make linux MYCFLAGS="-I$(cd ../fake;pwd)/include -fPIC" MYLDFLAGS="-L$(cd ../fake;pwd)/lib" MYLIBS=-lncurses
make install INSTALL_TOP=$(cd ../fake;pwd)
popd

gettext_src=${gettext_filename%.*.*}
pushd ${gettext_src}
./configure --prefix=$(cd ../fake;pwd)
make
make install
popd

# xcb和alsa都是跟系统深度绑定的，不宜下载源码单独编译，因此没有包含在编译脚本中。如果需要可以安装相应开发包，并去掉disable选项。
if [ "x${vlc_filename}" == 'x' ];then
  vlc_src=vlc
  pushd ${vlc_src}
  # vlc的C++11特性检测在部分发行版有缺陷，禁用bootstrap的自动检测
  sed -i 's/AX_CXX_COMPILE_STDCXX_11/dnl \0/' configure.ac
  ./bootstrap
else
  vlc_src=${vlc_filename%.*.*}
  pushd ${vlc_src}
fi
# 在所有Linux系统都存在这个编译错误，将bsd的strlcpy替换成等价的linux函数strncpy
sed -i s/strlcpy/strncpy/ src/input/subtitles.c
PKG_CONFIG_PATH=$(cd ../fake/lib/pkgconfig;pwd) CFLAGS="-I$(cd ../fake;pwd)/include -fPIC" CXXFLAGS="-std=gnu++11" LDFLAGS="-L$(cd ../fake;pwd)/lib" ./configure --prefix=$(cd ../..;pwd)/vlc_result --with-mad=$(cd ../fake;pwd) --with-a52=$(cd ../fake;pwd) LIBXML2_CFLAGS="-I $(cd ../fake/include/libxml2;pwd)" LUA_CFLAGS='-I$(cd ../fake;pwd)/include' LUA_LIBS='-L$(cd ../fake;pwd)/lib -llua' LUAC=$(cd ../fake;pwd)/bin/luac --disable-libgcrypt --disable-xcb --disable-alsa --enable-static
case ${dist} in
# 在Ubuntu系统不能自动链接math库，需要特别指定。
ubuntu16|ubuntu14)
  sed -i 's/^LIBS_a52.*/\0 -lm/' modules/Makefile
  ;;
esac
make
make install
popd

popd
