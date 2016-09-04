#!/usr/bin/env bash
mkdir build
cd build
cmake .. -DRAKNET_ENABLE_DLL=OFF -DRAKNET_ENABLE_SAMPLES=OFF -DCMAKE_BUILD_TYPE=Release
make -j2
make install
mkdir -p deb/usr/lib
mkdir -p deb/usr/share/raknet
mkdir -p deb/DEBIAN
cat > deb/DEBIAN/control << EOF
Package: libraknet-dev
Version: 4.081-1
Section: libdevel
Architecture: amd64
Priority: optional
Build-Depends: cmake
Origin: https://github.com/TES3MP/RakNet
Maintainer: Koncord < stas5978@gmail.com>
Description: RakNet is a cross-platform C++ and C# game networking engine.
 It is designed to be a high performance, easy to integrate,
 and complete solution for games and other applications.

EOF
cp Lib/LibStatic/libRakNetLibStatic.a deb/usr/lib ; chmod 0644 deb/usr/lib/libRakNetLibStatic.a
cp -r ../include deb/usr
cp ../LICENSE deb/usr/share/raknet ; chmod 0644 deb/usr/share/raknet/LICENSE
cp ../PATENTS deb/usr/share/raknet ; chmod 0644 deb/usr/share/raknet/PATENTS
cp -r ../Help deb/usr/share/raknet
chmod 0644 deb/usr/share/raknet/Help/*.*
chmod 0644 deb/usr/share/raknet/Help/Doxygen/*.*
chmod 0644 deb/usr/share/raknet/Help/Doxygen/html/*.*
chmod 0755 $(find deb/usr -type d)
cd deb
md5sum $(find ./ -type f | awk '!/^\.\/DEBIAN/ { print substr($0, 3) }') > DEBIAN/md5sums ; chmod 0644 DEBIAN/md5sums
cd ..
fakeroot dpkg-deb --build deb
lintian deb.deb
mv deb.deb libraknet-dev_4.081-1_amd64.deb
