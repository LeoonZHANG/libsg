#! /bin/bash
# �������ɵ��ļ��ڽű�ͬһ���vlc_resultĿ¼

# �κ���������Զ��˳��ű�
set -e

# �� "PROXY=���� ./vlc.sh" ����ʽָ���������δָ����ʹ�ô���
http_proxy=$PROXY
ftp_proxy=$PROXY

# ����ѹ�������Ŀ¼
mkdir -p tarball

# ����vlcԴ���
vlc_version='2.2.4'
vlc_url="http://get.videolan.org/vlc/2.2.4/vlc-${vlc_version}.tar.xz"
vlc_filename=`basename ${vlc_url}`
if [ ! -e "tarball/${vlc_filename}" ];then
  curl -# -fsSL ${vlc_url} > tarball/${vlc_filename}
fi

# ����xzԴ���
xz_version='5.2.2'
xz_url="http://tukaani.org/xz/xz-${xz_version}.tar.gz"
xz_filename=`basename ${xz_url}`
if [ ! -e "tarball/${xz_filename}" ];then
  curl -# -fsSL ${xz_url} > tarball/${xz_filename}
fi

# ����libxml2Դ���
libxml2_version='2.9.4'
libxml2_url="ftp://xmlsoft.org/libxml2/libxml2-${libxml2_version}.tar.gz"
libxml2_filename=`basename ${libxml2_url}`
if [ ! -e "tarball/${libxml2_filename}" ];then
  curl -# -fsSL ${libxml2_url} > tarball/${libxml2_filename}
fi


# ��������ļ�����������ļ�Ӱ��
rm -rf src
mkdir -p src/fake
pushd src

# ʹ��heredoc��������չ�ɶ���
cat << EOF
���ڽ�ѹԴ�룬���Է���һ��^_^
EOF

# ��ѹԴ���
tar -xvf ../tarball/${xz_filename}
tar -xvf ../tarball/${libxml2_filename}
tar -xvf ../tarball/${vlc_filename}

# ʹ��heredoc��������չ�ɶ���
cat << EOF
��ѹԴ����ɣ������޸������ھ�����Դ��·����`pwd`��
�޸���ϰ��������������
EOF
# �������������ִ��
read

xz_src=${xz_filename%.*.*}
pushd ${xz_src}
# ������Ҫָ������·��
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
# ��ʱ���ã��������˳�
# ���û�������
export CC="xcrun clang"
export CXX="xcrun clang++"
export OBJC="xcrun clang"
set +e
# ִ�б���ű�
../extras/package/macosx/build.sh
# �ָ��������������
set -e
# ��ȡҪ�滻���ļ�·��
libpath=$(dirname $(find .. -name libxml2.a))
cp ../../fake/lib/libxml2.{,l}a $libpath
# �ٴ�ִ�б���ű�
../extras/package/macosx/build.sh
cd ..
cp -r build ../../vlc_result
popd

popd
