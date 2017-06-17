#pragma once
#include <webrtc/media/base/videosinkinterface.h>
#include <webrtc/video_frame.h>
#include "webrtc/system_wrappers/include/critical_section_wrapper.h"
#include "scoped_ptr.h"

class VideoSinkProxy :public rtc::VideoSinkInterface<webrtc::VideoFrame>
{
public:
	VideoSinkProxy() :
		_locker(webrtc::CriticalSectionWrapper::CreateCriticalSection()) {
		_sink = nullptr;
	}
	void setSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink)
	{
		_locker->Enter();
		this->_sink = sink;
		_locker->Leave();
	}
	virtual void OnFrame(const webrtc::VideoFrame& frame) {
		_locker->Enter();
		if (this->_sink != nullptr) {
			this->_sink->OnFrame(frame);
		}
		_locker->Leave();
	}
private:
	rtc::VideoSinkInterface<webrtc::VideoFrame>* _sink;
	foxrtc::scoped_ptr<webrtc::CriticalSectionWrapper> _locker;

};