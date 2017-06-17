#pragma once
#include "foxrtc.h"
#include "scoped_ptr.h"
#include <webrtc/base/scoped_ref_ptr.h>
#include <webrtc/video_decoder.h>
#include <webrtc/base/task_queue.h>
#include <webrtc/base/logging.h>
#include <webrtc/base/logsinks.h>
#include <webrtc/voice_engine/voice_engine_impl.h>
#include <webrtc/call.h>
#include <webrtc/modules/audio_coding/codecs/builtin_audio_decoder_factory.h>
#include <webrtc/system_wrappers/include/critical_section_wrapper.h>
#include <webrtc/system_wrappers/include/rw_lock_wrapper.h>
#include <webrtc/system_wrappers/include/event_wrapper.h>
#include <webrtc/modules/video_render/video_render.h>
#include <webrtc/config.h>
#include <webrtc/logging/rtc_event_log/rtc_event_log.h>
#include <webrtc/media/engine/webrtcvoe.h>
#include <webrtc/modules/audio_coding/codecs/builtin_audio_decoder_factory.h>
#include <webrtc/base/asyncpacketsocket.h>
#include <webrtc/video_encoder.h>
#include <webrtc/video_decoder.h>
#include <webrtc/modules/video_coding/codec_database.h>
#include <webrtc/test/frame_generator_capturer.h>
#include <webrtc/modules/video_capture/video_capture_factory.h>
#include <webrtc/modules/video_capture/video_capture.h>
#include "video_sink_proxy.h"
#include "video_capture_source.h"
#include "video_process_bridge.h"
#include "encoder_stream_factory.h"

using namespace webrtc;
using namespace rtc;

class AudioLoopbackTransport;
class VideoLoopbackTransport;

class FoxrtcImpl:public Foxrtc
{
public:
	FoxrtcImpl();
	virtual ~FoxrtcImpl();
	virtual int Init(FoxrtcTransport* transport);
	virtual int Uninit();
	virtual int GetDeviceInfo();
	virtual int OpenCamera(int index);
	virtual int CloseCamera();
	virtual int CreateLocalAudioStream(unsigned int ssrc);
	virtual int DeleteLocalAudioStream();
	virtual int CreateRemoteAudioStream(unsigned int ssrc);
	virtual int DeleteRemoteAudioStream();
	virtual int CreateLocalVideoStream(int ssrc, void* view);
	virtual int DeleteLocalVideoStream();
	virtual int CreateRemoteVideoStream(int ssrc, void* view);
	virtual int DeleteRemoteVideoStream();
	virtual int IncomingData(const char* data, int len);

	Call* GetCall();
private:
	Call* _call = nullptr;
	webrtc::AudioSendStream* _audioSendStream = nullptr;
	webrtc::AudioReceiveStream* _audioReceiveStream = nullptr;
	webrtc::VideoSendStream* _videoSendStream = nullptr;
	webrtc::VideoReceiveStream* _videoReceiveStream = nullptr;

	int _audioSendChannelId = -1;
	int _audioReceiveChannelId = -1;
	int _videoSendChannelId = -1;
	int _videoReceiveChannelId = -1;

	webrtc::VideoCodec _videoCodec;

	VideoSinkProxy _videoSink;

	rtc::FileRotatingLogSink* _logsink = nullptr;
	webrtc::Atomic32* _stream_id = new Atomic32(0);
	rtc::scoped_refptr<webrtc::AudioDecoderFactory> _audioDecoderFactory = CreateBuiltinAudioDecoderFactory();

	//for test
	AudioLoopbackTransport* _audioSendTransport = nullptr;
	VideoLoopbackTransport* _videoSendTransport = nullptr;
};
