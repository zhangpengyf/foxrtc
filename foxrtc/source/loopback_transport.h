#pragma once
#include <webrtc/transport.h>
#include <webrtc/call.h>
#include "foxrtc_impl.h"

class AudioLoopbackTransport :public webrtc::Transport {
public:
	virtual bool SendRtp(const uint8_t* packet, size_t length, const webrtc::PacketOptions& options)
	{
		webrtc::Call* call = ((FoxrtcImpl*)&Foxrtc::Instance())->GetCall();
		rtc::PacketTime pTime = rtc::CreatePacketTime(0);
		webrtc::PacketReceiver::DeliveryStatus status = call->Receiver()->DeliverPacket(webrtc::MediaType::AUDIO, packet, length, webrtc::PacketTime(pTime.timestamp, pTime.not_before));
		assert(status == webrtc::PacketReceiver::DeliveryStatus::DELIVERY_OK);
		return true;
	}
	virtual bool SendRtcp(const uint8_t* packet, size_t length)
	{
		Call* call = ((FoxrtcImpl*)&Foxrtc::Instance())->GetCall();
		rtc::PacketTime pTime = rtc::CreatePacketTime(0);
		webrtc::PacketReceiver::DeliveryStatus status = call->Receiver()->DeliverPacket(webrtc::MediaType::AUDIO, packet, length, webrtc::PacketTime(pTime.timestamp, pTime.not_before));
		assert(status == webrtc::PacketReceiver::DeliveryStatus::DELIVERY_OK);
		return true;
	}
};

class VideoLoopbackTransport :public webrtc::Transport {
public:
	virtual bool SendRtp(const uint8_t* packet, size_t length, const webrtc::PacketOptions& options)
	{
		Call* call = ((FoxrtcImpl*)&Foxrtc::Instance())->GetCall();
		rtc::PacketTime pTime = rtc::CreatePacketTime(0);
		webrtc::PacketReceiver::DeliveryStatus status = call->Receiver()->DeliverPacket(webrtc::MediaType::VIDEO, packet, length, webrtc::PacketTime(pTime.timestamp, pTime.not_before));
		assert(status == webrtc::PacketReceiver::DeliveryStatus::DELIVERY_OK);
		return true;
	}
	virtual bool SendRtcp(const uint8_t* packet, size_t length)
	{
		Call* call = ((FoxrtcImpl*)&Foxrtc::Instance())->GetCall();
		rtc::PacketTime pTime = rtc::CreatePacketTime(0);
		webrtc::PacketReceiver::DeliveryStatus status = call->Receiver()->DeliverPacket(webrtc::MediaType::VIDEO, packet, length, webrtc::PacketTime(pTime.timestamp, pTime.not_before));
		assert(status == webrtc::PacketReceiver::DeliveryStatus::DELIVERY_OK);
		return true;
	}
};
