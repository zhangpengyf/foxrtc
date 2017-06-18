#pragma once

class FoxrtcTransport
{
public:
	FoxrtcTransport();
	virtual ~FoxrtcTransport();
	virtual int SendRtp(const char* data, int len);
	virtual int SendRtcp(const char* data, int len);
private:

};

class Foxrtc
{
public:
	Foxrtc();
	virtual ~Foxrtc();

	static Foxrtc& Instance();

	virtual int Init(FoxrtcTransport* transport) = 0;
	virtual int Uninit() = 0;

	virtual int GetDeviceInfo() = 0;

	virtual int OpenCamera(int index) = 0;
	virtual int CloseCamera() = 0;

	virtual int CreateLocalAudioStream(unsigned int ssrc) = 0;
	virtual int DeleteLocalAudioStream() = 0;
	virtual int CreateRemoteAudioStream(unsigned int ssrc) = 0;
	virtual int DeleteRemoteAudioStream() = 0;

	virtual int CreateLocalVideoStream(int ssrc, void* view) = 0;
	virtual int DeleteLocalVideoStream() = 0;
	virtual int CreateRemoteVideoStream(int ssrc, void* view) = 0;
	virtual int DeleteRemoteVideoStream() = 0;

	virtual int IncomingData(const char* data, int len) = 0;

};


