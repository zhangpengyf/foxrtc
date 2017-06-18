#ifndef ac_event_login_H__
#define ac_event_login_H__

#include "AC/Talk/Event/EventArgs.h"
namespace AC
{
	namespace Talk
	{
		namespace Event
		{
			class ACLoginStateEventArgs:public ACEventArgs
			{
				DECLARE_EVENTARGS(ACLoginStateEventArgs);
			public:
				ACServiceState OldState;
				ACServiceState NewState;
			};
			class ACLoginFailedEventArgs:public ACEventArgs
			{
			public:
				static const int RC_ERROR_CONFIG		=0;
				static const int RC_ERROR_NAME_PWD		=1;
				static const int RC_TIMEOUT				=2;
				static const int RC_NETWORK				=3;
				static const int RC_OTHER				=4;
				//static const int REASON_OTHER=
			public:
				DECLARE_EVENTARGS(ACLoginFailedEventArgs);
			public:
				ac_string Reason;
				ac_int32  ReasonCode;
				//http json {code:value}
				ac_int32  LoginCode;
			};
			class ACClientUpdateEventArgs:public ACEventArgs
			{
			public:
				DECLARE_EVENTARGS(ACClientUpdateEventArgs);
			public:
				ac_int32 UpdateType;
				ac_string UpdateVersion;
				ac_string UpdateUri;
				ac_uint64 FileSize;
				ac_string Md5;
				ac_string UpdateInfo;
			};
		}
	}
}
#endif // ac_event_login_H__
