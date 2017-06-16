webrtc android版编译步骤
下载ndk r12版本
使用r12版本ndk生成toolchain,命令为 ./build/tools/make-standalone-toolchain.sh --platform=android-16 --install-dir=./my-android-toolchain --stl=libcxx
把my-android-toolchain放到home目录~/my-android-toolchain
设置环境变量 export ANDROID_STANDALONE_TOOLCHAIN=~/my-android-toolchain
拷贝webrtc55版本同步后的chromium到webrtc里
修改文件acme/umcs2/webrtc/src/chromium/src/build/config/android/config.gni里面的ndk sdk目录为自己系统中的版本
