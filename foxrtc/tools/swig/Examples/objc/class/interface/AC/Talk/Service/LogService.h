#ifndef LogService_H__
#define LogService_H__
#include "AC/Talk/UserAgentTypes.h"
namespace AC
{
	namespace Talk
	{
		namespace Service
		{
			class LogService
			{
			public:
				static const ac_string CKEY_LOG_PATH;
			public:
				virtual void AddLogType(LogType type) = 0;
			    virtual	void RemoveLogType(LogType type) = 0;
				virtual void SetLogLevel(LogLevel level) = 0;
			};
		}
	}
}

#endif // LogService_H__
