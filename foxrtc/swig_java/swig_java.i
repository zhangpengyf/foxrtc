%module(directors="1") Foxrtc_android
%{
#include "../interface/foxrtc.h"
%}



%include "std_vector.i"
%include "std_string.i"
%include typemaps.i

%typemap(jni) void* "jobject"
%typemap(jtype) void* "Object"
%typemap(jstype) void* "Object"
%typemap(javain) void* "$javainput"


%typemap(jni) void* view "jobject"
%typemap(jtype) void* view "Object"
%typemap(jstype) void* view "android.view.SurfaceView"
%typemap(javain) void* view "$javainput"

%typemap(imtype) void* process "int"
%typemap(objctype) void* process "int"
%typemap(in) void* process
%{
    $1=(void*)$input;
%}
%typemap(objcin) void* process "$objcinput"


%include "../interface/foxrtc.h"