#!/bin/bash
set -e
PATH=$PATH:/c/Python27
if [ $# -lt 1 ];then
    echo "default build is Release"
    MODE=Release
    cd src
	export DEPOT_TOOLS_WIN_TOOLCHAIN=0
    gn gen out/win_$MODE --ide=vs2015 --args="is_debug=false is_component_build=true rtc_enable_protobuf=false  rtc_include_tests=false target_cpu=\"x86\""
    ninja -C out/win_$MODE
    cd -
else
    echo "build mode is Debug"
    MODE=Debug
    cd src
	export DEPOT_TOOLS_WIN_TOOLCHAIN=0
    gn gen out/win_$MODE --ide=vs2015 --args="is_debug=true is_component_build=true rtc_enable_protobuf=false  rtc_include_tests=false target_cpu=\"x86\""
    ninja -C out/win_$MODE
    cd -
fi

DST=../refs/pc/webrtc/$MODE
rm -rf $DST/*
if [ ! -d $DST ];then
    mkdir -p $DST
fi

#copy libs
for i in `find src/out/win_$MODE -name "*.lib"`
    do
	echo copy $i
	cp -f $i $DST/
    done
for i in `find src/out/win_$MODE -name "*.pdb"`
    do
	echo copy $i
	cp -f $i $DST/
    done
#copy vpx.o
for i in `find src/out/win_$MODE/obj/third_party/libvpx/ -name "libvpx_intrinsics_*"`
    do
	if [ -d $i ];then
		echo copy $i
		cp -f $i/*.obj $DST/
	fi
done
