#ifndef ACClassSession_H__
#define ACClassSession_H__
#include "AC/Talk/UserAgentTypes.h"
#include "AC/Talk/UserAgent.h"
namespace AC
{
	namespace Talk
	{
		namespace Session
		{


			enum ClassUserRole
			{
				ROLE_STUDENT = 0X0000,
				ROLE_TEACHER = 0X0001,
				ROLE_SELLER  = 0X0002
			};
			struct ClassUser
			{
				ac_id				UserId;
				ac_string			DisplayName;
				ClassUserRole		Role;
				ac_uint8			Right;
				ACClientType		ClientType;
				ac_uint32			MediaId;
			};
			struct ClassSessionInfo
			{
				ac_id				CourseId;
				ac_id				ClassId;

			};
			struct ClassUserInfo
			{
				ac_id				UserId;
				ac_string			DisplayName;	//nick name;
				ClassUserRole		Role;
			};
			struct SendImContext:public ActionContext
			{
				ac_string	Msg;
				ac_string	Option;
				ac_time		SendTime;
			};
			struct MakeUserOnMicAction:ActionContext
			{
				ac_id UserId;
			};
			struct MakeUserOffMicAction:ActionContext
			{
				ac_id UserId;
			};
			class ClassSession
			{
			public:
				//setting strings;
				const static ac_string SETTING_ALLOW_IM;
				const static ac_string SETTING_ALLOW_CHANGE_PAGE;

			public:
				static ClassSession* CreateSession(ClassSessionInfo& sessionInfo,ClassUserInfo& selfInfo, UserAgent* userAgent);
				static void	ReleaseSession(ac_uint32 id);
				static ClassSession* GetSession(ac_uint32 id);
				static bool HasSession(ac_uint32 id);
			public:
				virtual ~ClassSession(){}
			public:
				/**
				* 进入教室
				*
				*/
				virtual void Enter() = 0;
				/**
				* 离开教室
				*
				*/
				virtual void Leave() = 0;
				/**
				* 修改指定人权限
				*
				*/
				virtual void ChangeRight(ac_id uid,ac_uint8 right) = 0;
				/**
				* 发送即时消息到教室.
				*
				* @param context 即时消息内容
				*
				* @return 
				*/
				virtual void SendIm(SendImContext* context) = 0;
				/**
				* 设定教室设置.
				* 
				* 根据配置类型设定教室设置，见 #ClassSettingType
				* 
				* @param type 要设定的设置类型
				* @param value 设置类型新值
				*
				* @return true 成功，false 失败
				*/
				virtual ac_bool SetSetting(const ac_string& type,const ac_string& value) = 0;


				/**
				* 老师提问问题.
				*
				* @param question 问题内容 
				*
				* @return 
				*/
				virtual void Question(const ClassQuestion& question) = 0;

				/**
				* 学生回答问题.
				*
				* @param answer 问题答案 
				*
				* @return 
				*/
				virtual void Answer(const ClassAnswer& answer) = 0;
				/**
				* 获取教室设置.
				* 
				* 根据配置类型获取教室设置，见 #ClassSettingType
				* 
				* @param type 要获取的设置类型
				* @param value 传回当前设置值
				*
				* @return true 成功，false 失败 当前无配置
				*/
				virtual ac_bool GetSetting(const ac_string& type,ac_string& value) = 0;
				/**
				* 教室内是否有老师.
				*
				* @return true 存在，false 不存在
				*/
				virtual ac_bool HasTeacher() = 0;


				virtual const ClassUser* Teacher() = 0; 
				/**
				* 教室当前状态.
				*
				* 参考 #ACClassState
				*
				*/
				virtual ACClassState State() = 0;
				/**
				* 教室当前的课程ID
				*
				*/
				virtual ac_id CourseId() = 0;
				/**
				* 教室ID
				*
				*/
				virtual ac_id ClassId() = 0;
				/**
				* 
				* 
				* 跟课程无关，每个教室有独立的ID，用于事件分发，获取教室
				*
				*/
				virtual ac_uint32 Id() = 0;
				/**
				* 根据UserId获取教室成员信息
				*
				* @param userId 要获取的成员ID
				*
				* @return not null 成员信息。null，无此成员
				*/
				virtual const ClassUser* GetUser(ac_id userId) = 0;
				/**
				* 举手.
				*
				* @param context 上下文信息，用于状态回调
				*
				* @return 
				*/
				virtual void Handup(ActionContext* context) = 0;

				/**
				* 取消举手.
				*
				*
				* @return 
				*/
				virtual void Handdown(ActionContext* context) = 0;
				/**
				* 自己是否在 举手状态
				*
				*/
				virtual bool IsHandup() = 0;

				virtual void MakeUserOnMic(MakeUserOnMicAction* context) = 0;

				virtual void MakeUserOffMic(MakeUserOffMicAction* context) = 0;

				virtual void OperateWB(WBOperationData& data) = 0;

				virtual void MovePencil(WBPencilPosData& data) = 0;

				virtual void OperateDoc(WBDocumentData& data) = 0;

				/**
				* 教室内所有成员
				*
				*/
				virtual const std::vector<ClassUser>& AllUsers() = 0;
				/**
				 * 在麦人员列表
				 *
				 */
				virtual const std::vector<ClassUser>& OnMicUsers() = 0;
				/**
				 * 举手人员列表
				 *
				 */
				virtual const std::vector<ClassUser>& HandupUsers() = 0;
			};
		}
	}
}

#endif // ACClassSession_H__
