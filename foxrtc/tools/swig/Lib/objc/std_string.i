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
    int $1_length = [$input length];
    int $1_bufferSize = sizeof(char) * ($1_length + 1);
    char *$1_buffer = (char*)malloc($1_bufferSize);
    memset($1_buffer, 0, $1_bufferSize);
    int $1_i = 0;
    for (; $1_i < $1_length; ++$1_i)
        $1_buffer[$1_i] = [$$1_input characterAtIndex: $1_i];

    const char *$1_pstr = (const char *)$1_buffer;
    $1.assign($1_pstr);
    free($1_buffer); %}

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
   int $1_length = [$input length];
   int $1_bufferSize = sizeof(char) * ($1_length + 1);
   char *$1_buffer = (char*)malloc($1_bufferSize);
   memset($1_buffer, 0, $1_bufferSize);
   int $1_i = 0;
   for (; $1_i < $1_length; ++$1_i)
       $1_buffer[$1_i] = [$input characterAtIndex: $1_i];

   const char *$1_pstr = (const char *)$1_buffer;
   std::string $1_str($1_pstr);
   $1 = &$1_str;
   free($1_buffer); %}

%typemap(out) const string & 
%{ $result = [NSString stringWithUTF8String: $1->c_str()]; %}

%typemap(objcin) const string & "$objcinput"

%typemap(objcout) const string & {
    return $imcall;
  }
}
