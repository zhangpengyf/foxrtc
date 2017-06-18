#ifndef ServiceManager_H__
#define ServiceManager_H__
#include "AC/Talk/Service/ACService.h"
#include "AC/Talk/Service/ConfigService.h"
#include "AC/Talk/Service/CourseService.h"
#include "AC/Talk/Service/LogService.h"
namespace AC
{
	namespace Talk
	{
		class ServiceManager
		{
		public:
			virtual AC::Talk::Service::ACService*			ACService() = 0;
			virtual AC::Talk::Service::ConfigService*		UAConfigService() = 0;
			virtual AC::Talk::Service::ConfigService*		XmlConfigService() = 0;
			virtual AC::Talk::Service::CourseService*		CourseService() = 0;
			virtual AC::Talk::Service::LogService*			LogService() = 0;
		};
	}
}

#endif // ServiceManager_H__
