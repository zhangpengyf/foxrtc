#!/bin/bash

#quit when error happen
set -e

cd src
#generate a xcode project for read code
if [ ! -d "out/ios_read_code" ];then
    gn gen out/ios_read_code --ide=xcode --args='target_os="ios" target_cpu="arm64" is_component_build=false is_debug=true rtc_enable_protobuf=false  rtc_include_tests=false'
fi

if [ $# -lt 1 ];then
    echo "default build is Release"
    MODE=Release
    gn gen out/ios_arm64_$MODE --args='target_os="ios" target_cpu="arm64" is_component_build=false is_debug=false rtc_enable_protobuf=false  rtc_include_tests=false'
    gn gen out/ios_arm_$MODE --args='target_os="ios" target_cpu="arm" is_component_build=false is_debug=false rtc_enable_protobuf=false  rtc_include_tests=false'
    ninja -C out/ios_arm64_$MODE -k 500
    ninja -C out/ios_arm_$MODE -k 500
else
    echo "build mode is Debug"
    MODE=Debug
    gn gen out/ios_arm64_$MODE --args='target_os="ios" target_cpu="arm64" is_component_build=false is_debug=true rtc_enable_protobuf=false  rtc_include_tests=false'
    gn gen out/ios_arm_$MODE --args='target_os="ios" target_cpu="arm" is_component_build=false is_debug=true rtc_enable_protobuf=false  rtc_include_tests=false'
    ninja -C out/ios_arm64_$MODE -k 500
    ninja -C out/ios_arm_$MODE -k 500
fi

cd -

#generate libs
cd src/out/ios_arm64_$MODE
ALL_LIBS=`find . -name "*.a"`
cd -

DST=../refs/ios/webrtc/$MODE
rm -rf $DST/*
if [ ! -d $DST ];then
    mkdir -p $DST
fi

for i in $ALL_LIBS;do
    echo ${i##*/}
    LIBNAME=$(basename $i .a)
    LINK="$LINK -l${LIBNAME#*lib}"
    if [ -f src/out/ios_arm64_$MODE/$i ];then
        ARM64="src/out/ios_arm64_$MODE/$i"
	WEBRTC_ARM64="src/out/ios_arm64_$MODE/WebRTC.framework/WebRTC"
    fi
    if [ -f src/out/ios_arm_$MODE/$i ];then
        ARM="src/out/ios_arm_$MODE/$i"
	WEBRTC_ARM="src/out/ios_arm_$MODE/WebRTC.framework/WebRTC"
    fi
    lipo -create $ARM64 $ARM $SIM -output $DST/${i##*/}
done

cp -rf src/out/ios_arm64_$MODE/WebRTC.framework $DST/
lipo -create $WEBRTC_ARM64 $WEBRTC_ARM -output $DST/WebRTC.framework/WebRTC


echo "Link flag is:"
echo $LINK
		   
