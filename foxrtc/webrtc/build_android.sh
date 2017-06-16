#!/bin/bash

if [ -z "$ANDROID_STANDALONE_TOOLCHAIN" ]; then 
    echo "must set ANDROID_STANDALONE_TOOLCHAIN" 
    exit 1
fi

cd src

if [ $# -lt 1 ];then
    echo "default build is Release"
    MODE=Release
    gn gen out/android_$MODE --args='target_os="android" target_cpu="arm" is_component_build=false is_debug=false rtc_enable_protobuf=false  rtc_include_tests=false'
    ninja -C out/android_$MODE -k 500
else
    echo "build mode is Debug"
    MODE=Debug
    gn gen out/android_$MODE --args='target_os="android" target_cpu="arm" is_component_build=false is_debug=true rtc_enable_protobuf=false  rtc_include_tests=false'
    ninja -C out/android_$MODE -k 500
fi

cd -

#copy libs
DST=../refs/android/webrtc/$MODE
rm -rf $DST/*
if [ ! -d $DST ];then
    mkdir -p $DST
fi

for i in `find src/out/android_$MODE -name "*.a"`
    do
    echo $i
    cp $i $DST/
done


cd $DST/
rm -rf libvideo_capture.a
ls | awk '{print "$ANDROID_STANDALONE_TOOLCHAIN/bin/arm-linux-androideabi-ar -x "$1"; $ANDROID_STANDALONE_TOOLCHAIN/bin/arm-linux-androideabi-ar -q liball.a *.o ; rm -rf *.o"}' | sh
