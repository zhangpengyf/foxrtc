#ifndef ac_class_events_H__
#define ac_class_events_H__
#include "AC/Talk/Event/EventArgs.h"
namespace AC
{
	namespace Talk
	{

		namespace Event
		{

			/**
			* 发送即时消息动作状态变化事件.
			*
			*/
			class ClassSendImStateEventArgs:public ActionStateEventArgs
			{
				DECLARE_EVENTARGS(ClassSendImStateEventArgs);
			};
			/**
			* 收到教室内消息事件
			*
			*/
			class ClassRecvImEventArgs:public ACEventArgs
			{
				DECLARE_EVENTARGS(ClassRecvImEventArgs);
				ac_id		From;
				ac_time		SendTime;
				ac_string	Option;
				ac_string	Message;
			};
			class ClassEnterFailedEventArgs:public ACEventArgs
			{
			public:
				DECLARE_EVENTARGS(ClassEnterFailedEventArgs)
			public:
				ac_int32 Code;
			};
			/**
			* 用户操作.
			* 
			*
			*/
			enum Operation
			{
				/**
				* 增加
				*/
				OP_ADD,
				/**
				* 移除
				*/
				OP_REMOVE,
				/**
				* 权限变更
				*
				*/
				OP_CHANGE_RIGHT,
				/**
				* 更新
				*/
				OP_UPDATE
			};
			/**
			* 教室内用户操作基类
			*
			*/
			class ClassUserEventArgs:public ACEventArgs
			{
			public:
				/**
				* 操作类型
				*/
				Operation Op;
				/**
				* 涉及的用户ID
				*/
				std::vector<ac_id> Users;
			};
			/**
			* 教室状态变化事件.
			*
			*/
			class ClassStateEventArgs:public ACEventArgs
			{
			public:
				DECLARE_EVENTARGS(ClassStateEventArgs);
				/**
				* 新的教室状态
				*
				*/
				ACClassState State;
			};
			/**
			* 教室内学生信息变化事件.
			*
			* 当有学生进入，退出教室，学生状态(昵称...)变化时触发
			*
			*/
			class ClassUserChangedEventArgs:public ClassUserEventArgs
			{
			public:
				DECLARE_EVENTARGS(ClassUserChangedEventArgs);
			public:
			};




			class ClassRightChangedEventArgs:public ACEventArgs
			{
			public:
				DECLARE_EVENTARGS(ClassRightChangedEventArgs);
			public:
				ac_uint8 OldRight;
				ac_uint8 NewRight;
			};

			/**
			* 教室小测试事件
			*
			*/
			class ClassQuestionEventArgs:public ACEventArgs
			{
				DECLARE_EVENTARGS(ClassQuestionEventArgs);
				ClassQuestion Question;
			};
			/**
			* 小测试答题事件
			*
			*/
			class ClassAnswerEventArgs:public ACEventArgs
			{
				DECLARE_EVENTARGS(ClassAnswerEventArgs);
				ClassAnswer Answer;
			};
			/**
			* 老师进入事件.
			*
			*/
			class ClassTeacherEnterEventArgs:public ACEventArgs
			{
				DECLARE_EVENTARGS(ClassTeacherEnterEventArgs);

			};
			/**
			* 老师离开事件
			*
			*/
			class ClassTeacherLeaveEventArgs:public ACEventArgs 
			{
				DECLARE_EVENTARGS(ClassTeacherLeaveEventArgs);
			};

			/**
			* 教室设置改变事件
			*
			*/
			class ClassSettingChangedEventArgs:public ACEventArgs
			{
				DECLARE_EVENTARGS(ClassSettingChangedEventArgs);

				/**
				* 配置类型
				*
				*/
				ac_string Type;

				/**
				* 配置旧内容
				*
				*/
				ac_string OldValue;

				/**
				* 配置变更后的内容
				*
				*/
				ac_string NewValue;
			};

						/**
			* 教室内举手学生变化事件.
			*
			* 当有学生举手，取消举手时，举手列表变更时
			*
			*/
			class ClassHandupUserChangedEventArgs:public ClassUserEventArgs
			{
				DECLARE_EVENTARGS(ClassHandupUserChangedEventArgs);
			};
			/**
			* 教室内在麦用户变化事件.
			* 
			* 当学生上麦，下麦时
			*
			*/
			class ClassOnMicUserChangedEventArgs:public ClassUserEventArgs
			{
				DECLARE_EVENTARGS(ClassOnMicUserChangedEventArgs);
			};

			class ClassMakeUserOnMicEventArgs:public ActionStateEventArgs
			{
				DECLARE_EVENTARGS(ClassMakeUserOnMicEventArgs);
			};

			class ClassMakeUserOffMicEventArgs:public ActionStateEventArgs
			{
				DECLARE_EVENTARGS(ClassMakeUserOffMicEventArgs);
			};

			class ClassHandupStateEventArgs:public ActionStateEventArgs
			{
				DECLARE_EVENTARGS(ClassHandupStateEventArgs);
			};
			class ClassHanddownStateEventArgs:public ActionStateEventArgs
			{
				DECLARE_EVENTARGS(ClassHanddownStateEventArgs);
			};

			/**
			* 收到白板鼠标位置事件
			*
			*/
			class ClassWBPosEventArgs:public ACEventArgs
			{
				DECLARE_EVENTARGS(ClassWBPosEventArgs);
				WBPencilPosData Data;
			};
			/**
			* 收到白板文档状态事件
			*
			*/
			class ClassWBDocumentEventArgs:public ACEventArgs
			{
				DECLARE_EVENTARGS(ClassWBDocumentEventArgs);
				WBDocumentData Data;
			};
			/**
			* 收到白板数据内容
			*
			*/
			class ClassWBDataEventArgs:public ACEventArgs
			{
				DECLARE_EVENTARGS(ClassWBDataEventArgs);
				WBOperationData Data;
			};
		}
	}
}

#endif // ac_class_events_H__
