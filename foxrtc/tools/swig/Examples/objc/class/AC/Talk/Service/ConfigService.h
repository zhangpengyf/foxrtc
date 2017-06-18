#ifndef ConfigService_H__
#define ConfigService_H__
#include "AC/Talk/UserAgentTypes.h"
namespace AC
{
	namespace Talk
	{
		namespace Service
		{
			class ConfigService
			{
			public:
				virtual void SetString(const ac_string& key,const ac_string& value) = 0;
				virtual ac_string GetString(const ac_string& key,const ac_string& defaultValue) = 0;

				virtual void SetBool(const ac_string& key,bool value) = 0;
				virtual bool GetBool(const ac_string& key,bool defaultValue) = 0;

				virtual void SetInt(const ac_string& key,int32_t value) = 0;
				virtual int32_t GetInt(const ac_string& key,int32_t defaultValue) = 0;

				virtual void SetUInt(const ac_string& key,uint32_t value) = 0;
				virtual uint32_t GetUInt(const ac_string& key,uint32_t defaultValue) = 0;

				virtual void SetInt64(const ac_string& key,int64_t value) = 0;
				virtual int64_t GetInt64(const ac_string& key,int64_t defaultValue) = 0;

				virtual void SetUInt64(const ac_string& key,uint64_t value) = 0;
				virtual uint64_t GetUInt64(const ac_string& key,uint64_t defaultValue) = 0;
			};
		}
	}
}

#endif // ConfigService_H__
