#pragma once

class VideoCaptureSource
	:public rtc::VideoSourceInterface<webrtc::VideoFrame>
	, public webrtc::VideoCaptureDataCallback
	, public webrtc::VideoCaptureFeedBack
{
public:
	VideoCaptureSource() :
		_locker(webrtc::CriticalSectionWrapper::CreateCriticalSection())
	{
	}
	int StartCapture(int index, const webrtc::VideoCaptureCapability &capability);
	int StopCapture();
	int DeliverFrame(webrtc::VideoFrame& frame);
protected:
	virtual void OnIncomingCapturedFrame(const int32_t id,
		const webrtc::VideoFrame& videoFrame);
	virtual void OnCaptureDelayChanged(const int32_t id,
		const int32_t delay) {
		_delay = delay;
	};
	virtual void OnCaptureFrameRate(const int32_t id,
		const uint32_t frameRate) {
		_frameRate = frameRate;
	};
	virtual void OnNoPictureAlarm(const int32_t id,
		const webrtc::VideoCaptureAlarm alarm) {};
public:
	virtual void AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink,
		const rtc::VideoSinkWants& wants);
	// RemoveSink must guarantee that at the time the method returns,
	// there is no current and no future calls to VideoSinkInterface::OnFrame.
	virtual void RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink);
private:
	rtc::scoped_refptr<webrtc::VideoCaptureModule> _module;
	std::vector<rtc::VideoSinkInterface<webrtc::VideoFrame>*> _sinks;
	foxrtc::scoped_ptr<webrtc::CriticalSectionWrapper> _locker;
	uint32_t _frameRate;
	int32_t _delay;
};