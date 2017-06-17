#include "foxrtc_impl.h"
#include "loopback_transport.h"

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

static Foxrtc& Instance()
{
	static Foxrtc* instance = nullptr;
	if (instance == nullptr)
	{
		instance = new FoxrtcImpl();
	}
	return *instance;
}

FoxrtcImpl::FoxrtcImpl()
{
}

FoxrtcImpl::~FoxrtcImpl()
{
	delete _stream_id;
	delete _audioDecoderFactory;
}

Call* FoxrtcImpl::GetCall()
{
	return _call;
}

int FoxrtcImpl::Init(FoxrtcTransport* transport)
{
    if (_call != nullptr) {
        return -1;
    }
    LogMessage::ConfigureLogging("tstamp thread info debug");
    _logsink = new rtc::FileRotatingLogSink("./", "foxrtc", 10 * 1024 * 1024, 10);
    _logsink->Init();
    LogMessage::AddLogToStream(_logsink, LS_INFO);
    
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
    VOE.BASE->Init(nullptr, nullptr, _audioDecoderFactory);
    
    VIE.CAPTURE_SOURCE = new VideoCaptureSource();
    VIE.CAMERA_SOURCE = new VideoCaptureSource();
    VOE.AUDIO_STATE = AudioState::Create(audioStateConfig);
    callConfig.audio_state = VOE.AUDIO_STATE;
    _call = Call::Create(callConfig);
    VIE.DEVICE = VideoCaptureFactory::CreateDeviceInfo(0);
    VOE.LOCAL_ID = VOE.BASE->CreateChannel();
    rtc::VideoSinkWants vsw;
    VIE.CAMERA_SOURCE->AddOrUpdateSink(&VIE.PREVIEW_SINK, vsw);
    return 0;
}

int FoxrtcImpl::Uninit()
{
	if (_call != nullptr) {
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
		if (_logsink != nullptr) {
			LogMessage::RemoveLogToStream(_logsink);
			_logsink = nullptr;
		}
	}
	return 0;
}

int FoxrtcImpl::GetDeviceInfo()
{
	return 0;
}

int FoxrtcImpl::OpenCamera(int index)
{
	return 0;
}
int FoxrtcImpl::CloseCamera()
{
	return 0;
}

int FoxrtcImpl::CreateLocalAudioStream(unsigned int ssrc)
{
	if (_audioSendStream != nullptr) {
		return -1;
	}
    _audioSendTransport = new AudioLoopbackTransport();
	AudioSendStream::Config streamConfig(_audioSendTransport);
	streamConfig.voe_channel_id = VOE.LOCAL_ID;
	streamConfig.rtp.ssrc = ssrc;
	VOE.LOCAL_SSRC = ssrc;
	_audioSendStream =
		_call->CreateAudioSendStream(std::move(streamConfig));
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
	_audioSendStream->Start();
	return 0;
}

int FoxrtcImpl::DeleteLocalAudioStream()
{
	if (_audioSendStream != nullptr)
    {
        return 0;
    }
	_audioSendStream->Stop();
	_call->DestroyAudioSendStream(_audioSendStream);
	return 0;

}


int FoxrtcImpl::CreateRemoteAudioStream(unsigned int ssrc)
{
	_audioReceiveChannelId = VOE.BASE->CreateChannel();
	AudioReceiveStream::Config streamConfig;
	streamConfig.rtp.local_ssrc = VOE.LOCAL_SSRC;
	streamConfig.rtp.remote_ssrc = ssrc;
	streamConfig.rtcp_send_transport = _audioSendTransport;
	streamConfig.voe_channel_id = _audioReceiveChannelId;
	streamConfig.decoder_factory = _audioDecoderFactory;
	_audioReceiveStream = _call->CreateAudioReceiveStream(std::move(streamConfig));
	_audioReceiveStream->Start();
	return 0;
}

int FoxrtcImpl::DeleteRemoteAudioStream()
{
    if (_audioReceiveStream == nullptr) {
        return -1;
    }
	_audioReceiveStream->Stop();
	VOE.BASE->StopReceive(_audioReceiveChannelId);
	VOE.BASE->StopPlayout(_audioReceiveChannelId);
	VOE.BASE->DeleteChannel(_audioReceiveChannelId);
	_call->DestroyAudioReceiveStream(_audioReceiveStream);
	return 0;
}

int FoxrtcImpl::CreateLocalVideoStream(int ssrc, void* view)
{
	if (_videoSendStream != nullptr) {
		return -1;
	}
    _videoSendTransport = new VideoLoopbackTransport();
	VideoSendStream::Config streamConfig(_videoSendTransport);
	streamConfig.encoder_settings.payload_name = "VP9";
	streamConfig.encoder_settings.payload_type = 121;
	streamConfig.rtp.max_packet_size = 1350;
	streamConfig.encoder_settings.encoder = webrtc::VideoEncoder::Create(VideoEncoder::kVp9);
    streamConfig.rtp.ssrcs.push_back(ssrc);
    VIE.LOCAL_SSRC = ssrc;
    //VideoEncoderConfig
    webrtc::VideoEncoderConfig encoder_config;
    webrtc::VCMCodecDataBase::Codec(webrtc::kVideoCodecVP8, &_videoCodec);
    encoder_config.encoder_specific_settings = new rtc::RefCountedObject<webrtc::VideoEncoderConfig::Vp8EncoderSpecificSettings>(_videoCodec.codecSpecific.VP8);
    encoder_config.encoder_specific_settings->FillEncoderSpecificSettings(&_videoCodec);
    encoder_config.content_type = webrtc::VideoEncoderConfig::ContentType::kRealtimeVideo;
    encoder_config.number_of_streams = 1;
    encoder_config.video_stream_factory = new rtc::RefCountedObject<EncoderStreamFactory>(_videoCodec.plName, _videoCodec.qpMax, _videoCodec.maxFramerate, false, false);
    
	_videoSendStream = _call->CreateVideoSendStream(
		std::move(streamConfig), std::move(encoder_config));
	_videoSendStream->SetSource(VIE.CAPTURE_SOURCE);
	_videoSendStream->Start();
	return 0;
}

int FoxrtcImpl::DeleteLocalVideoStream()
{
    if (_videoSendStream != nullptr) {
        return -1;
    }
	_videoSendStream->Stop();
	_call->DestroyVideoSendStream(_videoSendStream);
	return 0;
}

int FoxrtcImpl::CreateRemoteVideoStream(int ssrc, void* view)
{
	VideoReceiveStream::Config streamConfig(_videoSendTransport);
	streamConfig.renderer = &_videoSink;
	streamConfig.rtp.remote_ssrc = ssrc;
	streamConfig.rtp.local_ssrc = VIE.LOCAL_SSRC;
    webrtc::VideoReceiveStream::Decoder decoder;
    decoder.decoder = webrtc::VideoDecoder::Create(webrtc::VideoDecoder::DecoderType::kVp8);
    streamConfig.decoders.push_back(decoder);
	streamConfig.rtp.rtcp_xr.receiver_reference_time_report = true;
	streamConfig.rtp.nack.rtp_history_ms = 2000;
	_videoReceiveStream = _call->CreateVideoReceiveStream(std::move(streamConfig));
	_videoReceiveStream->Start();
	return 0;
}
int FoxrtcImpl::DeleteRemoteVideoStream()
{
	if (_videoReceiveStream == nullptr) {
		return -1;
	}
	_videoReceiveStream->Stop();
	_call->DestroyVideoReceiveStream(_videoReceiveStream);
	return 0;
}

int FoxrtcImpl::IncomingData(const char* data, int len)
{
	if (_call != nullptr) {
        webrtc::PacketTime pt;
		_call->Receiver()->DeliverPacket(MediaType::ANY, (const uint8_t*)data, len, pt);
		return 0;
	}
	return 0;
}

