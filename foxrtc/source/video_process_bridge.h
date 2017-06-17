#pragma once
#include <webrtc/video_frame.h>
#include <webrtc/modules/video_processing/include/video_processing.h>

class VideoProcessBrigde :public rtc::VideoSourceInterface<webrtc::VideoFrame>,
	public rtc::VideoSinkInterface<webrtc::VideoFrame>
{
public:
	VideoProcessBrigde() :
		_locker(webrtc::CriticalSectionWrapper::CreateCriticalSection())
		, _processing(webrtc::VideoProcessing::Create()) {

	}
	void SetPaused(bool enable) {
		_paused = enable;
	}
	virtual void OnFrame(const webrtc::VideoFrame& frame) {
		if (_paused)
		{
			return;
		}
		const webrtc::VideoFrame* newFrame = _processing->PreprocessFrame(frame);
		if (newFrame == nullptr)
			return;
		webrtc::CriticalSectionScoped ls(_locker.get());
		for (auto item : _sinks) {
			item->OnFrame(*newFrame);
		}
	}
	virtual void AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink,
		const rtc::VideoSinkWants& wants) {
		webrtc::CriticalSectionScoped ls(_locker.get());
		for (auto item : _sinks) {
			if (item == sink) {
				return;
			}
		}
		_sinks.push_back(sink);
	}
	// RemoveSink must guarantee that at the time the method returns,
	// there is no current and no future calls to VideoSinkInterface::OnFrame.
	virtual void RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink) {
		webrtc::CriticalSectionScoped ls(_locker.get());
		auto item = _sinks.begin();
		for (; item != _sinks.end(); item++) {
			if (*item == sink) {
				_sinks.erase(item);
				return;
			}
		}
	}
public:
	webrtc::VideoProcessing* Processing() {
		return _processing;
	}
private:
	webrtc::VideoProcessing* _processing = nullptr;
	std::vector<rtc::VideoSinkInterface<webrtc::VideoFrame>*> _sinks;
	foxrtc::scoped_ptr<webrtc::CriticalSectionWrapper> _locker;
	bool _paused = false;
};