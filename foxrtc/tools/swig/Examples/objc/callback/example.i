/* File : example.i */
%module(directors="1") example
%{
#include "example.h"
#include "UserAgentTypes.h"
#include "EventArgs.h"

%}

%include "std_string.i"
%include "std_vector.i"
/* turn on director wrapping Callback */
%feature("director") Callback;

%include "UserAgentTypes.h"
%include "example.h"
%include "EventArgs.h"

namespace std
{
  %template(CallerVector) vector<Caller>;
}
