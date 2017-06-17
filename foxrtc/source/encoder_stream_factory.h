#pragma once
#include <webrtc/config.h>
#include <webrtc/video_encoder.h>

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
		stream.target_bitrate_bps = stream.max_bitrate_bps = 500 * 1000;
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