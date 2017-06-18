/* -----------------------------------------------------------------------------
 * std_string.i
 *
 * Typemaps for std::string and const std::string&
 * These are mapped to an Objective-C NSString*
 *
 * To use non-const std::string references use the following %apply.
 * %apply const std::string & {std::string &};
 * ----------------------------------------------------------------------------- */

%{
#include <string>
%}

namespace std {

%naturalvar string;

class string;

// string
%typemap(imtype) string "NSString*"
%typemap(objctype) string "NSString*"

%typemap(in) string 
%{ if(!$input) {
	NSException* anException = [NSException exceptionWithName:@"NullPointerException" 
	reason:@"null std::string" userInfo:nil];
	@throw anException;
    } 
    const char *$1_pstr = $input.UTF8String;
    $1.assign($1_pstr);
   %}

%typemap(out) string 
%{ $result = [NSString stringWithUTF8String: $1.c_str()]; %}

%typemap(objcin) string "$objcinput"

%typemap(objcout) string {
    return $imcall;
  }

// const string &
%typemap(imtype) const string & "NSString*"
%typemap(objctype) const string & "NSString*"

%typemap(in) const string &
%{ if(!$input) {
	NSException* anException = [NSException exceptionWithName:@"NullPointerException" 
	reason:@"null std::string" userInfo:nil];
	@throw anException;
   }
   const char *$1_pstr = $input.UTF8String;
   std::string $1_str($1_pstr);
   $1 = &$1_str;
   %}

%typemap(out) const string & 
%{ $result = [NSString stringWithUTF8String: $1->c_str()]; %}

%typemap(objcin) const string & "$objcinput"

%typemap(objcout) const string & {
    return $imcall;
  }
}
