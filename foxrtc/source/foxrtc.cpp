#include "foxrtc.h"
#include "webrtc/base/scoped_ref_ptr.h"
#include "webrtc/video_encoder.h"
#include "webrtc/video_decoder.h"
#include "webrtc/base/task_queue.h"
#include "webrtc/base/logging.h"
#include "webrtc/base/logsinks.h"
#include "webrtc/modules/video_processing/include/video_processing.h"
#include "webrtc/voice_engine/voice_engine_impl.h"
#include "webrtc/call.h"
#include "webrtc/modules/audio_coding/codecs/builtin_audio_decoder_factory.h"
#include "webrtc/system_wrappers/include/critical_section_wrapper.h"
#include "webrtc/system_wrappers/include/rw_lock_wrapper.h"
#include "webrtc/system_wrappers/include/event_wrapper.h"
#include "scoped_ptr.h"
#include "webrtc/modules/video_capture/video_capture_factory.h"
#include "webrtc/modules/video_render/video_render.h"
using namespace webrtc;
using namespace rtc;

class VideoSinkProxy:public rtc::VideoSinkInterface<VideoFrame>
{
public:
    VideoSinkProxy():
    _locker(webrtc::CriticalSectionWrapper::CreateCriticalSection()){
        _sink = nullptr;
    }
    void setSink(rtc::VideoSinkInterface<VideoFrame>* sink)
    {
        _locker->Enter();
        this->_sink = sink;
        _locker->Leave();
    }
    virtual void OnFrame(const VideoFrame& frame){
        _locker->Enter();
        if(this->_sink!=nullptr){
            this->_sink->OnFrame(frame);
        }
        _locker->Leave();
    }
private:
    rtc::VideoSinkInterface<VideoFrame>* _sink;
    foxrtc::scoped_ptr<webrtc::CriticalSectionWrapper> _locker;
    
};
struct AudioSendStreamInfo {

};
struct AudioReceiveStreamInfo {

};

struct VideoSendStreamInfo {

};
struct VideoReceiveStreamInfo {
    
};

class VideoCaptureSource
:public rtc::VideoSourceInterface<VideoFrame>
,public VideoCaptureDataCallback
,public VideoCaptureFeedBack
{
public:
    VideoCaptureSource():
    _locker(webrtc::CriticalSectionWrapper::CreateCriticalSection())
    {
    }
    int StartCapture(int index,const webrtc::VideoCaptureCapability &capability);
    int StopCapture();
    int DeliverFrame(VideoFrame& frame);
protected:
    virtual void OnIncomingCapturedFrame(const int32_t id,
                                         const VideoFrame& videoFrame);
    virtual void OnCaptureDelayChanged(const int32_t id,
                                       const int32_t delay){
        _delay = delay;
    };
    virtual void OnCaptureFrameRate(const int32_t id,
                                    const uint32_t frameRate){
        _frameRate = frameRate;
    };
    virtual void OnNoPictureAlarm(const int32_t id,
                                  const VideoCaptureAlarm alarm){};
public:
    virtual void AddOrUpdateSink(rtc::VideoSinkInterface<VideoFrame>* sink,
                                 const rtc::VideoSinkWants& wants);
    // RemoveSink must guarantee that at the time the method returns,
    // there is no current and no future calls to VideoSinkInterface::OnFrame.
    virtual void RemoveSink(rtc::VideoSinkInterface<VideoFrame>* sink);
private:
    rtc::scoped_refptr<VideoCaptureModule> _module;
    std::vector<rtc::VideoSinkInterface<VideoFrame>*> _sinks;
    foxrtc::scoped_ptr<webrtc::CriticalSectionWrapper> _locker;
    uint32_t _frameRate;
    int32_t _delay;
};

class VideoProcessBrigde:public rtc::VideoSourceInterface<VideoFrame>,
public rtc::VideoSinkInterface<VideoFrame>
{
public:
    VideoProcessBrigde():
    _locker(webrtc::CriticalSectionWrapper::CreateCriticalSection())
    ,_processing(VideoProcessing::Create()){
        
    }
    void SetPaused(bool enable) {
        _paused = enable;
    }
    virtual void OnFrame(const VideoFrame& frame){
        if (_paused)
        {
            return;
        }
        const VideoFrame* newFrame = _processing->PreprocessFrame(frame);
        if(newFrame == nullptr)
        return;
        webrtc::CriticalSectionScoped ls(_locker.get());
        for(auto item:_sinks){
            item->OnFrame(*newFrame);
        }
    }
    virtual void AddOrUpdateSink(rtc::VideoSinkInterface<VideoFrame>* sink,
                                 const rtc::VideoSinkWants& wants){
        webrtc::CriticalSectionScoped ls(_locker.get());
        for(auto item:_sinks){
            if(item == sink){
                return;
            }
        }
        _sinks.push_back(sink);
    }
    // RemoveSink must guarantee that at the time the method returns,
    // there is no current and no future calls to VideoSinkInterface::OnFrame.
    virtual void RemoveSink(rtc::VideoSinkInterface<VideoFrame>* sink){
        webrtc::CriticalSectionScoped ls(_locker.get());
        auto item = _sinks.begin();
        for (; item!=_sinks.end(); item++) {
            if(*item == sink){
                _sinks.erase(item);
                return;
            }
        }
    }
public:
    VideoProcessing* Processing(){
        return _processing;
    }
private:
    VideoProcessing* _processing = nullptr;
    std::vector<rtc::VideoSinkInterface<VideoFrame>*> _sinks;
    foxrtc::scoped_ptr<webrtc::CriticalSectionWrapper> _locker;
    bool _paused = false;
};
typedef std::map<int, VideoSendStreamInfo*> LOCAL_VIDEO_MAP;
typedef std::map<int, VideoReceiveStreamInfo*> REMOTE_VIDEO_MAP;
typedef std::map<int, AudioReceiveStreamInfo*> REMOTE_AUDIO_MAP;
struct UMCS_VideoEngine {
    REMOTE_VIDEO_MAP         REMOTE_VIDEOS;
    VideoSendStreamInfo*     CAMERA_VIDEO;
    VideoSendStreamInfo*     CAPTURE_VIDEO;
    VideoCaptureModule::DeviceInfo* DEVICE;
    VideoCaptureSource*      CAMERA_SOURCE;
    VideoCaptureSource*      CAPTURE_SOURCE;
    VideoProcessBrigde*      CAMERA_PROCESS_SOURCE;
    webrtc::Transport*       TRANSPORT;
    VideoRender*             PREVIEW_RENDER;
    VideoSinkProxy           PREVIEW_SINK;
}VIE;
struct UMCS_AudioEngine{
    VoiceEngine*             ENGINE;
    VoEVolumeControl*        VOLUME_CONTROL;
    VoEHardware*             HARDWARE;
    VoECodec*                CODEC;
    VoEBase*                 BASE;
    VoERTP_RTCP*             RTP_RTCP;
    int                      LOCAL_ID;
    int                      LOCAL_SSRC;
    VoENetwork*              NETWORK;
    VoEAudioProcessing*      AUDIO_PROC;
    VoEExternalMedia*        EXTERNAL_MEDIA;
    AudioSendStreamInfo*     LOCAL_AUDIO;
    REMOTE_AUDIO_MAP         REMOTE_AUDIOS;
    rtc::scoped_refptr<AudioState> AUDIO_STATE;
    VoiceEngine*             AUDIO_ENGINE;
    webrtc::Transport*       TRANSPORT;
}VOE;

Call* g_call = nullptr;
rtc::FileRotatingLogSink* g_logsink = nullptr;
webrtc::Atomic32* g_stream_id = new Atomic32(0);
rtc::scoped_refptr<webrtc::AudioDecoderFactory> g_audioDecoderFactory =
CreateBuiltinAudioDecoderFactory();

int Fox_Init()
{
    if (g_call != nullptr) {
        return -1;
    }
    LogMessage::ConfigureLogging("tstamp thread info debug");
    g_logsink = new rtc::FileRotatingLogSink("./", "foxrtc", 10 * 1024 * 1024, 10);
    g_logsink->Init();
    LogMessage::AddLogToStream(g_logsink, LS_INFO);
    
    Call::Config callConfig;
    VOE.ENGINE = VoiceEngine::Create();
    webrtc::AudioState::Config audioStateConfig;
    audioStateConfig.voice_engine = VOE.ENGINE;
    VOE.AUDIO_ENGINE = audioStateConfig.voice_engine;
    VOE.CODEC = VoECodec::GetInterface(VOE.ENGINE);
    VOE.BASE = VoEBase::GetInterface(VOE.ENGINE);
    VOE.VOLUME_CONTROL = VoEVolumeControl::GetInterface(VOE.ENGINE);
    VOE.HARDWARE = VoEHardware::GetInterface(VOE.ENGINE);
    VOE.RTP_RTCP = VoERTP_RTCP::GetInterface(VOE.ENGINE);
    VOE.AUDIO_PROC = VoEAudioProcessing::GetInterface(VOE.ENGINE);
    VOE.EXTERNAL_MEDIA = VoEExternalMedia::GetInterface(VOE.ENGINE);
    VOE.BASE->Init(nullptr, nullptr, g_audioDecoderFactory);
    
    VIE.CAPTURE_SOURCE = new VideoCaptureSource();
    VIE.CAMERA_SOURCE = new VideoCaptureSource();
    VOE.AUDIO_STATE = AudioState::Create(audioStateConfig);
    callConfig.audio_state = VOE.AUDIO_STATE;
    g_call = Call::Create(callConfig);
    VIE.DEVICE = VideoCaptureFactory::CreateDeviceInfo(0);
    VOE.LOCAL_ID = VOE.BASE->CreateChannel();
    rtc::VideoSinkWants vsw;
    VIE.CAMERA_SOURCE->AddOrUpdateSink(&VIE.PREVIEW_SINK, vsw);
    return 0;
}

int Fox_Uninit()
{
	if (g_call != nullptr) {
		if (VIE.DEVICE != nullptr) {
			delete VIE.DEVICE;
			VIE.DEVICE = nullptr;
		}
		if (VIE.CAPTURE_SOURCE != nullptr) {
			VIE.CAPTURE_SOURCE->StopCapture();
			delete VIE.CAPTURE_SOURCE;
			VIE.CAPTURE_SOURCE = nullptr;
		}
		if (VIE.CAMERA_SOURCE != nullptr) {
			VIE.CAMERA_SOURCE->StopCapture();
			delete VIE.CAMERA_SOURCE;
			VIE.CAMERA_SOURCE = nullptr;
		}
		if (VOE.AUDIO_ENGINE != nullptr) {
			VOE.BASE->StopSend(VOE.LOCAL_ID);
			VOE.BASE->Release();
			VOE.BASE = nullptr;
			VOE.CODEC->Release();
			VOE.CODEC = nullptr;
			VOE.RTP_RTCP->Release();
			VOE.RTP_RTCP = nullptr;
			VOE.VOLUME_CONTROL->Release();
			VOE.VOLUME_CONTROL = nullptr;
			VOE.HARDWARE->Release();
			VOE.HARDWARE = nullptr;
			VOE.AUDIO_PROC->Release();
			VOE.AUDIO_PROC = nullptr;
			VOE.EXTERNAL_MEDIA->Release();
			VOE.EXTERNAL_MEDIA = nullptr;
			VoiceEngine::Delete(VOE.AUDIO_ENGINE);
			VOE.AUDIO_ENGINE = nullptr;
			VOE.AUDIO_STATE = nullptr;
			VOE.LOCAL_ID = -1;
		}
		if (g_logsink != nullptr) {
			LogMessage::RemoveLogToStream(g_logsink);
			g_logsink = nullptr;
		}
	}
	return 0;
}

int Fox_GetDeviceInfo()
{
	return 0;
}

int Fox_OpenCamera(int index)
{
	return 0;
}
int Fox_CloseCamera()
{
	return 0;
}

int Fox_SelectMicrophone(int index)
{
	return 0;
}
int Fox_SelectSpeaker(int index)
{
	return 0;
}

int Fox_SetMicrophoneVolume(unsigned int volume)
{
	return 0;
}
int Fox_SetPlayoutVolume(unsigned int volume)
{
	return 0;
}

int Fox_CreateLocalAudioStream(unsigned int ssrc)
{
	if (VOE.LOCAL_AUDIO != nullptr) {
		return -1;
	}
	AudioSendStream::Config streamConfig(&info->transport);
	streamConfig.voe_channel_id = VOE.LOCAL_ID;
	streamConfig.rtp.ssrc = ssrc;
	VOE.LOCAL_SSRC = ssrc;
	AudioSendStream* stream =
		g_call->CreateAudioSendStream(std::move(streamConfig));
	info->stream = stream;
	VOE.LOCAL_AUDIO = info;
	webrtc::CodecInst audioCodec;
	UMCS_GetAudioCodec(audioCodec);
	VOE.CODEC->SetSendCodec(VOE.LOCAL_ID, audioCodec);
	VOE.AUDIO_PROC->EnableHighPassFilter(true);
	VOE.CODEC->SetVADStatus(VOE.LOCAL_ID, true, kVadAggressiveMid);
#ifndef UMCS_IOS
	VOE.AUDIO_PROC->SetNsStatus(true, kNsVeryHighSuppression);
#ifdef UMCS_ANDROID
	VOE.AUDIO_PROC->SetEcStatus(true, kEcAecm);
#else
	VOE.AUDIO_PROC->SetEcStatus(true, kEcAec);
#endif
#endif
	stream->Start();
	*sid = id;
	return 0;
}

int Fox_DeleteLocalAudioStream()
{
	if (VOE.LOCAL_AUDIO == nullptr)
		return 0;
	VOE.LOCAL_AUDIO->stream->Stop();
	g_call->DestroyAudioSendStream(VOE.LOCAL_AUDIO->stream);
	delete VOE.LOCAL_AUDIO;
	VOE.LOCAL_AUDIO = nullptr;
	return 0;

}

int Fox_CreateRemoteAudioStream(unsigned int ssrc)
{
	AudioReceiveStreamInfo* info = new AudioReceiveStreamInfo(id, remoteSsrc);

	info->channelId = VOE.BASE->CreateChannel();
	AudioReceiveStream::Config streamConfig;
	streamConfig.rtp.local_ssrc = localSsrc;
	streamConfig.rtp.remote_ssrc = remoteSsrc;
	streamConfig.rtcp_send_transport = &info->transport;
	streamConfig.voe_channel_id = info->channelId;
	streamConfig.decoder_factory = g_audioDecoderFactory;
	AudioReceiveStream* stream =
		g_call->CreateAudioReceiveStream(std::move(streamConfig));
	info->stream = stream;
	VOE.REMOTE_AUDIOS[id] = info;
	stream->Start();
	*sid = id;
	return 0;
}

int Fox_DeleteRemoteAudioStream()
{
	UMCS_CHECK_INIT;
	auto item = VOE.REMOTE_AUDIOS.find(sid);
	if (item == VOE.REMOTE_AUDIOS.end()) {
		return -1;
	}
	item->second->stream->Stop();
	int channelId = item->second->channelId;
	VOE.BASE->StopReceive(channelId);
	VOE.BASE->StopPlayout(channelId);
	VOE.BASE->DeleteChannel(item->second->channelId);
	g_call->DestroyAudioReceiveStream(item->second->stream);
	delete item->second;
	VOE.REMOTE_AUDIOS.erase(item);
	return 0;
}

int Fox_CreateLocalVideoStream(int ssrc, void* view)
{
	UMCS_CHECK_INIT;
	if (VIE.CAPTURE_VIDEO != nullptr) {
		return -1;
	}
	int id = g_stream_id->operator++();
	VideoSendStreamInfo* info = new VideoSendStreamInfo(id, ssrc);
	VideoSendStream::Config streamConfig(&info->transport);
	streamConfig.encoder_settings.payload_name = "VP9";
	streamConfig.encoder_settings.payload_type = 121;
	streamConfig.rtp.max_packet_size = 1350;

	streamConfig.encoder_settings.encoder =
		webrtc::VideoEncoder::Create(VideoEncoder::kVp9);
	VideoEncoderConfig encodeConfig;
	UMCS_ConfigCameraCodec(encodeConfig);
	VideoSendStream* stream = g_call->CreateVideoSendStream(
		std::move(streamConfig), std::move(encodeConfig));
	stream->SetSource(VIE.CAPTURE_SOURCE);
	info->stream = stream;
	VIE.CAPTURE_VIDEO = info;
	stream->Start();
	*sid = id;
	return 0;
}

int Fox_DeleteLocalVideoStream()
{
	UMCS_CHECK_INIT;
	if (VIE.CAPTURE_VIDEO == nullptr) {
		return 0;
	}
	VIE.CAPTURE_VIDEO->stream->Stop();
	g_call->DestroyVideoSendStream(VIE.CAPTURE_VIDEO->stream);
	delete VIE.CAPTURE_VIDEO;
	VIE.CAPTURE_VIDEO = nullptr;
	return 0;
}

int Fox_CreateRemoteVideoStream(int ssrc, void* view)
{
	UMCS_CHECK_INIT;
	int id = getNewId();
	VideoReceiveStreamInfo* info = new VideoReceiveStreamInfo(id, remoteSsrc);
	VideoReceiveStream::Config streamConfig(&info->transport);
	streamConfig.renderer = &info->sink;
	streamConfig.rtp.remote_ssrc = remoteSsrc;
	streamConfig.rtp.local_ssrc = localSsrc;
	streamConfig.decoders = UMCS_GetVideoDecoders();
	streamConfig.rtp.rtcp_xr.receiver_reference_time_report = true;
	streamConfig.rtp.nack.rtp_history_ms = 2000;
	VideoReceiveStream* stream =
		g_call->CreateVideoReceiveStream(std::move(streamConfig));
	info->stream = stream;
	VIE.REMOTE_VIDEOS[id] = info;
	stream->Start();
	*sid = id;
	return 0;
}
int Fox_DeleteRemoteVideoStream()
{
	UMCS_CHECK_INIT;
	auto stream = VIE.REMOTE_VIDEOS.find(sid);
	if (stream == VIE.REMOTE_VIDEOS.end()) {
		return -1;
	}
	if (stream->second->render != nullptr) {
		UMCS_StopRemoteRender(sid);
	}
	stream->second->stream->Stop();
	g_call->DestroyVideoReceiveStream(stream->second->stream);
	delete stream->second;
	VIE.REMOTE_VIDEOS.erase(stream);
	return 0;
}

int Fox_InsertMediaData(char* data, int len)
{
	if (g_call != nullptr) {
		PacketTime pt;
		g_call->Receiver()->DeliverPacket(MediaType::ANY, (const uint8_t*)data, len,
			pt);
		return 0;
	}
	return 0;
}

