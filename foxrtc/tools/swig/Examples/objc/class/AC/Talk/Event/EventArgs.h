#ifndef ac_event_H__
#define ac_event_H__
#include "AC/Talk/UserAgentTypes.h"

#define DECLARE_EVENTARGS(name)	\
			public:\
			name(ac_uint32 id)\
			{\
			_id = id;\
			}\
			static ac_string TypeName()\
			{\
			return #name;\
			}\
			virtual ac_string EventName()\
			{\
			return name::TypeName();\
			}



namespace AC
{
	namespace Talk
	{
		namespace Event
		{
			class ACEventArgs
			{
			public:
				static ac_string TypeName(){return "ACEventArgs";}
			public:
				ACEventArgs()
				{
					_id = 0;
					_isProcessed = true;
				}
			public:
				ac_uint32 Id() const
				{
					return _id;
				}
				ac_bool IsProcessed() const
				{
					return _isProcessed;
				}
				ac_bool MakeProcessed()
				{
					_isProcessed = true;
				}
				virtual ac_string EventName() = 0;
			protected:
				ac_uint32 _id;
				ac_bool _isProcessed;
			};
			/**
			* 执行动作后动作变化通知.
			*
			*/
			class ActionStateEventArgs:public ACEventArgs
			{
			public:
				/**
				* 当前状态信息,参考 #ActionStatus
				*
				*/
				ActionStatus Status;
				/**
				* 执行动作时传入的上下文信息
				*
				*/
				ContextToken Token;
			};
		}
	}
}

#endif // ac_event_H__
