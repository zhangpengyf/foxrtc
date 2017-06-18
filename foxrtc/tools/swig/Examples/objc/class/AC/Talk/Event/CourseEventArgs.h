#ifndef CourseEventArgs_H__
#define CourseEventArgs_H__
#include "AC/Talk/Event/EventArgs.h"
namespace AC
{
	namespace Talk
	{
		namespace Event
		{
			class CourseUpdatedEventArgs:public ACEventArgs
			{
			public:
				DECLARE_EVENTARGS(CourseUpdatedEventArgs);
			};
		}
	}
}

#endif // CourseEventArgs_H__
