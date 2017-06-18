%module(directors="1") foxrtc
%{
#include "../interface/foxrtc.h"
%}


// const DataWrapper &
%typemap(imtype) const DataWrapper& "NSData*"
%typemap(objctype) const DataWrapper& "NSData*"
%typemap(objcin) const DataWrapper& "$objcinput"

%typemap(in) const DataWrapper&
%{ 
   int $1_length = $input.length;
   const void* $1_data = $input.bytes;

   DataWrapper $1_dw;
   $1_dw.len = $1_length;
   $1_dw.data = $1_data;
   $1 = &$1_dw;
 %}

%typemap(imtype) DataWrapper "NSData*"
%typemap(objctype) DataWrapper "NSData*"
%typemap(out) DataWrapper
%{ 
	$result = [[[NSData alloc]initWithBytes:$1.data length:$1.len]autorelease];
%}


%typemap(objcout) DataWrapper {
    return $imcall;
}

%include "std_vector_u.i"
%include "std_string_u.i"
%feature("director") EngineEventCallback;
%include typemaps.i

%typemap(objctype) void* view "RenderView*"

%typemap(objcin) void* view "(__bridge void*)$objcinput"

%typemap(imtype) void* process "int"
%typemap(objctype) void* process "int"
%typemap(in) void* process
%{
    $1=(void*)$input;
%}
%typemap(objcin) void* process "$objcinput"

%include "../interface/foxrtc.h"


