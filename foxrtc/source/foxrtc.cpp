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
#include "webrtc/modules/video_render/video_render.h"+
#include "webrtc/config.h"
#include "webrtc/logging/rtc_event_log/rtc_event_log.h"
#include "webrtc/media/engine/webrtcvoe.h"
#include "webrtc/modules/audio_coding/codecs/builtin_audio_decoder_factory.h"
#include "webrtc/base/asyncpacketsocket.h"
#include "webrtc/video_encoder.h"
#include "webrtc/video_decoder.h"
#include "webrtc/modules/video_coding/codec_database.h"
#include "webrtc/test/frame_generator_capturer.h"
#include "webrtc/modules/video_capture/video_capture_factory.h"
#include "webrtc/modules/video_capture/video_capture.h"


using namespace webrtc;
using namespace rtc;

Call* g_call = nullptr;
webrtc::AudioSendStream* g_audioSendStream = nullptr;
webrtc::AudioReceiveStream* g_audioReceiveStream = nullptr;
webrtc::VideoSendStream* g_videoSendStream = nullptr;
webrtc::VideoReceiveStream* g_videoReceiveStream = nullptr;

int g_audioSendChannelId = -1;
int g_audioReceiveChannelId = -1;
int g_videoSendChannelId = -1;
int g_videoReceiveChannelId = -1;

webrtc::VideoCodec g_videoCodec;

class EncoderStreamFactory : public webrtc::VideoEncoderConfig::VideoStreamFactoryInterface {
public:
    EncoderStreamFactory(std::string codec_name,
                         int max_qp,
                         int max_framerate,
                         bool is_screencast,
                         bool conference_mode)
    : codec_name_(codec_name),
    max_qp_(max_qp),
    max_framerate_(max_framerate),
    is_screencast_(is_screencast),
    conference_mode_(conference_mode) {}
    
private:
    std::vector<webrtc::VideoStream> CreateEncoderStreams(
                                                          int width,
                                                          int height,
                                                          const webrtc::VideoEncoderConfig& encoder_config) override {
        RTC_DCHECK(encoder_config.number_of_streams > 1 ? !is_screencast_ : true);
        
        
        webrtc::VideoStream stream;
        stream.width = width;
        stream.height = height;
        stream.max_framerate = max_framerate_;
        stream.min_bitrate_bps = 100 * 1000;
        stream.target_bitrate_bps = stream.max_bitrate_bps = 500*1000;
        stream.max_qp = max_qp_;
        
        
        std::vector<webrtc::VideoStream> streams;
        streams.push_back(stream);
        return streams;
    }
    
    const std::string codec_name_;
    const int max_qp_;
    const int max_framerate_;
    const bool is_screencast_;
    const bool conference_mode_;
};



class AudioLoopbackTransport:public webrtc::Transport{
public:
    virtual bool SendRtp(const uint8_t* packet,size_t length,const webrtc::PacketOptions& options)
    {
        rtc::PacketTime pTime = rtc::CreatePacketTime(0);
        webrtc::PacketReceiver::DeliveryStatus status = g_call->Receiver()->DeliverPacket(webrtc::MediaType::AUDIO, packet, length, webrtc::PacketTime(pTime.timestamp, pTime.not_before));
        assert(status == webrtc::PacketReceiver::DeliveryStatus::DELIVERY_OK);
        return true;
    }
    virtual bool SendRtcp(const uint8_t* packet, size_t length)
    {
        rtc::PacketTime pTime = rtc::CreatePacketTime(0);
        webrtc::PacketReceiver::DeliveryStatus status = g_call->Receiver()->DeliverPacket(webrtc::MediaType::AUDIO, packet, length, webrtc::PacketTime(pTime.timestamp, pTime.not_before));
        assert(status == webrtc::PacketReceiver::DeliveryStatus::DELIVERY_OK);
        return true;
    }
};

class VideoLoopbackTransport:public webrtc::Transport{
public:
    virtual bool SendRtp(const uint8_t* packet,size_t length,const webrtc::PacketOptions& options)
    {
        rtc::PacketTime pTime = rtc::CreatePacketTime(0);
        webrtc::PacketReceiver::DeliveryStatus status = g_call->Receiver()->DeliverPacket(webrtc::MediaType::VIDEO, packet, length, webrtc::PacketTime(pTime.timestamp, pTime.not_before));
        assert(status == webrtc::PacketReceiver::DeliveryStatus::DELIVERY_OK);
        return true;
    }
    virtual bool SendRtcp(const uint8_t* packet, size_t length)
    {
        rtc::PacketTime pTime = rtc::CreatePacketTime(0);
        webrtc::PacketReceiver::DeliveryStatus status = g_call->Receiver()->DeliverPacket(webrtc::MediaType::VIDEO, packet, length, webrtc::PacketTime(pTime.timestamp, pTime.not_before));
        assert(status == webrtc::PacketReceiver::DeliveryStatus::DELIVERY_OK);
        return true;
    }
};

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

VideoSinkProxy g_videoSink;

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

struct UMCS_VideoEngine {
    VideoCaptureModule::DeviceInfo* DEVICE;
    VideoCaptureSource*      CAMERA_SOURCE;
    VideoCaptureSource*      CAPTURE_SOURCE;
    VideoProcessBrigde*      CAMERA_PROCESS_SOURCE;
    webrtc::Transport*       TRANSPORT;
    VideoRender*             PREVIEW_RENDER;
    VideoSinkProxy           PREVIEW_SINK;
    int                      LOCAL_SSRC;
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
    rtc::scoped_refptr<AudioState> AUDIO_STATE;
    VoiceEngine*             AUDIO_ENGINE;
    webrtc::Transport*       TRANSPORT;
}VOE;

AudioLoopbackTransport* g_audioSendTransport = nullptr;
VideoLoopbackTransport* g_videoSendTransport = nullptr;

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
	if (g_audioSendStream != nullptr) {
		return -1;
	}
    g_audioSendTransport = new AudioLoopbackTransport();
	AudioSendStream::Config streamConfig(g_audioSendTransport);
	streamConfig.voe_channel_id = VOE.LOCAL_ID;
	streamConfig.rtp.ssrc = ssrc;
	VOE.LOCAL_SSRC = ssrc;
	g_audioSendStream =
		g_call->CreateAudioSendStream(std::move(streamConfig));
	//VOE.CODEC->SetSendCodec(VOE.LOCAL_ID, audioCodec);
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
	g_audioSendStream->Start();
	return 0;
}

int Fox_DeleteLocalAudioStream()
{
	if (g_audioSendStream != nullptr)
    {
        return 0;
    }
	g_audioSendStream->Stop();
	g_call->DestroyAudioSendStream(g_audioSendStream);
	return 0;

}

int Fox_CreateRemoteAudioStream(unsigned int ssrc)
{
	g_audioReceiveChannelId = VOE.BASE->CreateChannel();
	AudioReceiveStream::Config streamConfig;
	streamConfig.rtp.local_ssrc = VOE.LOCAL_SSRC;
	streamConfig.rtp.remote_ssrc = ssrc;
	streamConfig.rtcp_send_transport = g_audioSendTransport;
	streamConfig.voe_channel_id = g_audioReceiveChannelId;
	streamConfig.decoder_factory = g_audioDecoderFactory;
	g_audioReceiveStream = g_call->CreateAudioReceiveStream(std::move(streamConfig));
	g_audioReceiveStream->Start();
	return 0;
}

int Fox_DeleteRemoteAudioStream()
{
    if (g_audioReceiveStream == nullptr) {
        return -1;
    }
	g_audioReceiveStream->Stop();
	VOE.BASE->StopReceive(g_audioReceiveChannelId);
	VOE.BASE->StopPlayout(g_audioReceiveChannelId);
	VOE.BASE->DeleteChannel(g_audioReceiveChannelId);
	g_call->DestroyAudioReceiveStream(g_audioReceiveStream);
	return 0;
}

int Fox_CreateLocalVideoStream(int ssrc, void* view)
{
	if (g_videoSendStream != nullptr) {
		return -1;
	}
    g_videoSendTransport = new VideoLoopbackTransport();
	VideoSendStream::Config streamConfig(g_videoSendTransport);
	streamConfig.encoder_settings.payload_name = "VP9";
	streamConfig.encoder_settings.payload_type = 121;
	streamConfig.rtp.max_packet_size = 1350;
	streamConfig.encoder_settings.encoder = webrtc::VideoEncoder::Create(VideoEncoder::kVp9);
    streamConfig.rtp.ssrcs.push_back(ssrc);
    VIE.LOCAL_SSRC = ssrc;
    //VideoEncoderConfig
    webrtc::VideoEncoderConfig encoder_config;
    webrtc::VCMCodecDataBase::Codec(webrtc::kVideoCodecVP8, &g_videoCodec);
    encoder_config.encoder_specific_settings = new rtc::RefCountedObject<webrtc::VideoEncoderConfig::Vp8EncoderSpecificSettings>(g_videoCodec.codecSpecific.VP8);
    encoder_config.encoder_specific_settings->FillEncoderSpecificSettings(&g_videoCodec);
    encoder_config.content_type = webrtc::VideoEncoderConfig::ContentType::kRealtimeVideo;
    encoder_config.number_of_streams = 1;
    encoder_config.video_stream_factory = new rtc::RefCountedObject<EncoderStreamFactory>(g_videoCodec.plName, g_videoCodec.qpMax, g_videoCodec.maxFramerate, false, false);
    
	g_videoSendStream = g_call->CreateVideoSendStream(
		std::move(streamConfig), std::move(encoder_config));
	g_videoSendStream->SetSource(VIE.CAPTURE_SOURCE);
	g_videoSendStream->Start();
	return 0;
}

int Fox_DeleteLocalVideoStream()
{
    if (g_videoSendStream != nullptr) {
        return -1;
    }
	g_videoSendStream->Stop();
	g_call->DestroyVideoSendStream(g_videoSendStream);
	return 0;
}

int Fox_CreateRemoteVideoStream(int ssrc, void* view)
{
	VideoReceiveStream::Config streamConfig(g_videoSendTransport);
	streamConfig.renderer = &g_videoSink;
	streamConfig.rtp.remote_ssrc = ssrc;
	streamConfig.rtp.local_ssrc = VIE.LOCAL_SSRC;
    webrtc::VideoReceiveStream::Decoder decoder;
    decoder.decoder = webrtc::VideoDecoder::Create(webrtc::VideoDecoder::DecoderType::kVp8);
    streamConfig.decoders.push_back(decoder);
	streamConfig.rtp.rtcp_xr.receiver_reference_time_report = true;
	streamConfig.rtp.nack.rtp_history_ms = 2000;
	g_videoReceiveStream = g_call->CreateVideoReceiveStream(std::move(streamConfig));
	g_videoReceiveStream->Start();
	return 0;
}
int Fox_DeleteRemoteVideoStream()
{
	if (g_videoReceiveStream == nullptr) {
		return -1;
	}
	g_videoReceiveStream->Stop();
	g_call->DestroyVideoReceiveStream(g_videoReceiveStream);
	return 0;
}

int Fox_InsertMediaData(char* data, int len)
{
	if (g_call != nullptr) {
        webrtc::PacketTime pt;
		g_call->Receiver()->DeliverPacket(MediaType::ANY, (const uint8_t*)data, len, pt);
		return 0;
	}
	return 0;
}

