#!/bin/bash
set -e
if [ $# -lt 1 ];then
    echo "default build is Release"
    MODE=Release
    cd src
    if [ ! -d "out/mac_$MODE" ];then
    gn gen out/mac_$MODE --ide=xcode --args='is_debug=false  rtc_enable_protobuf=false  rtc_include_tests=false'
	fi
    ninja -C out/mac_$MODE call video_capture_internal_impl metrics_default field_trial_default rtc_base -k 500
    cd -
else
    echo "build mode is Debug"
    MODE=Debug
    cd src
    if [ ! -d "out/mac_$MODE" ];then
    gn gen out/mac_$MODE --ide=xcode --args='is_debug=true  rtc_enable_protobuf=false  rtc_include_tests=false'
    fi
    ninja -C out/mac_$MODE call video_capture_internal_impl metrics_default field_trial_default rtc_base -k 500
    cd -
fi
#copy libs


DST=../refs/mac/webrtc/$MODE
rm -rf $DST/*
if [ ! -d $DST ];then
    mkdir -p $DST
fi

for i in `find src/out/mac_$MODE -name "*.a"`
    do
	echo $i
	cp $i $DST/
done
#copy vpx.o
for i in `find src/out/mac_$MODE/obj/third_party/libvpx/ -name "libvpx_intrinsics_*"`
    do
	if [ -d $i ];then
		echo $i
		cp $i/*.o ../refs/mac/webrtc/
	fi
done
