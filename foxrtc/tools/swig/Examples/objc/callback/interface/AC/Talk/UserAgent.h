#ifndef UserAgent_H__
#define UserAgent_H__
#include "AC/Talk/Event/EventArgs.h"
#include "AC/Talk/ServiceManager.h"
namespace AC
{
	namespace Talk
	{
		class UITask
		{
		public:
			virtual void Execute(){};
		    virtual	~UITask(){};
		};
		class UserAgentCallback
		{
		public:
			virtual void DispatchEvent(AC::Talk::Event::ACEventArgs* eventArgs){}
			virtual void ExecuteUITask(AC::Talk::UITask* task){}
		};
		class UserAgentConfig
		{
		public:
			UserAgentCallback*		Callback;
			ac_string				XmlConfigPath;
		};
		class UserAgent
		{
		public:
			virtual ~UserAgent(){};
		public:
			static const ac_string CKEY_UA_HOST;
		public:
			static AC::Talk::UserAgent* CreateUserAgent(AC::Talk::UserAgentConfig* config);
			static void ReleaseUserAgent(AC::Talk::UserAgent* userAgent);
		public:
			virtual AC::Talk::ServiceManager* ServiceManager() = 0;
		};
	}
}

#endif // UserAgent_H__
