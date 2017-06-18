#ifndef CourseService_H__
#define CourseService_H__
#include "AC/Talk/UserAgentTypes.h"
namespace AC
{
	namespace Talk
	{
		namespace Service
		{
			class Course
			{
			public:
				virtual ac_uint64 CourseId()const = 0;
				virtual ac_uint64 TeacherId()const = 0;
				virtual ac_string StartTime()const = 0;
				virtual ac_string CourseName()const = 0;
				virtual ac_string TeacherName()const = 0;
				virtual ac_int32  CourseTypeId() const = 0;
				virtual ac_uint64 ClassId() const = 0;

				virtual ac_string GetProperty(const ac_string& key,const ac_string& defaultValue)const = 0;
				virtual ac_int32 GetProperty(const ac_string& key,const ac_int32& defaultValue)const = 0;
				virtual ac_bool GetProperty(const ac_string& key,const ac_bool& defaultValue)const = 0;

				virtual Course* Clone()const = 0;
			};
			class CourseService
			{
			public:
				static const ac_string CKEY_COURSE_HOST;
				static const ac_string CKEY_COURSE_URI;
			public:
				virtual const std::vector<Course*>& StuReservedCourses()= 0;
				virtual const std::vector<Course*>& StuOpenCourses()= 0;
				virtual const std::vector<Course*>& StuExperienceCourses()= 0;

				virtual const std::vector<Course*>& TeaPrivateCourses() = 0;
				virtual const std::vector<Course*>& TeaOpenCourses() = 0;
				virtual const std::vector<Course*>& TeaSmallCourses() = 0;
				virtual const std::vector<Course*>& TeaExperienceCourses() = 0;
				virtual const std::vector<Course*>& TeaPsoCourses() = 0;

				virtual bool Start() = 0;
				virtual void Stop() = 0;
				virtual bool Subscribe() = 0;
				virtual void UnSubscribe() = 0;
			};
		}
	}
}

#endif // CourseService_H__
