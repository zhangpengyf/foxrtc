#ifndef UserAgentCallbackWin_H__
#define UserAgentCallbackWin_H__
#include "AC/Talk/UserAgent.h"

namespace AC
{
	namespace Talk
	{
		class UserAgentCallbackWin:public UserAgentCallback
		{
		public:
			static UserAgentCallbackWin* Default();
		public:
			virtual void DispatchEvent(AC::Talk::Event::ACEventArgs* eventArgs) = 0;
			virtual void ExecuteUITask(AC::Talk::UITask* task) = 0;
		};
	}
}

#endif // UserAgentCallbackWin_H__
