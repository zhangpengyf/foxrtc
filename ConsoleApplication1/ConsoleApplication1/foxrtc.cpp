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
using namespace webrtc;
using namespace rtc;

Call* g_call = nullptr;
rtc::FileRotatingLogSink* g_logsink = nullptr;
webrtc::Atomic32* g_stream_id = new Atomic32(0);
rtc::scoped_refptr<webrtc::AudioDecoderFactory> g_audioDecoderFactory =
CreateBuiltinAudioDecoderFactory();

int Fox_Init()
{
	if (g_call == nullptr) {
		UMCS_CONFIG = *config;
		LogMessage::ConfigureLogging(configFromLogLevel(UMCS_CONFIG.traceFilter));
		if (UMCS_CONFIG.traceFile[0] != '\0') {
			char path[UMCS_MAX_PATH_LEN] = { 0 };
			memcpy(path, UMCS_CONFIG.traceFile, UMCS_MAX_PATH_LEN);
			std::string dir;
			std::string prefix;
			for (int i = UMCS_MAX_PATH_LEN - 2; i >= 0; i--) {
				if (path[i] == '/' || path[i] == '\\') {
					prefix = &path[i + 1];
					path[i] = '\0';
					dir = path;
					break;
				}
			}
			g_logsink =
				new rtc::FileRotatingLogSink(dir, prefix, 10 * 1024 * 1024, 10);
			g_logsink->Init();
			LogMessage::AddLogToStream(g_logsink, LS_INFO);
		}
		UMCS_INFO("umcs init");
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
	return -1;
}
int Fox_Uninit()
{
	if (g_call != nullptr) {
		LIVE_Stop();
		UMCS_StopRecordCall();
		UMCS_StopPlayFile();
		UMCS_EnableMicTest(false);
		UMCS_StopPreview();
		std::vector<int> remoteAudios;
		for (auto audio : VOE.REMOTE_AUDIOS) {
			remoteAudios.push_back(audio.first);
		}
		for (auto sid : remoteAudios) {
			UMCS_DeleteRemoteAudio(sid);
		}
		std::vector<int> remoteVideos;
		for (auto video : VIE.REMOTE_VIDEOS) {
			remoteVideos.push_back(video.first);
		}
		for (auto sid : remoteVideos) {
			UMCS_DeleteRemoteVideo(sid);
		}
		UMCS_DeleteLocalAudio();
		UMCS_DeleteCameraVideo();
		UMCS_DeleteCaptureVideo();
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
		memset(&UMCS_CONFIG, 0, sizeof(UMCS_CONFIG));
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
	UMCS_CHECK_INIT;
	if (VOE.LOCAL_AUDIO != nullptr) {
		return -1;
	}
	int id = getNewId();
	AudioSendStreamInfo* info = new AudioSendStreamInfo(id, ssrc);
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
	UMCS_CHECK_INIT;
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

