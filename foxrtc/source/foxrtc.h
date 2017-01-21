#pragma once

void (outgoingvideodata)(const char* data, int len, bool isRtcp);
void (outgoingvideodata)(const char* data, int len, bool isRtcp);

int Fox_Init();
int Fox_Uninit();

int Fox_GetDeviceInfo();

int Fox_OpenCamera(int index);
int Fox_CloseCamera();

int Fox_SelectMicrophone(int index);
int Fox_SelectSpeaker(int index);

int Fox_SetMicrophoneVolume(unsigned int volume);
int Fox_SetPlayoutVolume(unsigned int volume);

int Fox_CreateLocalAudioStream(unsigned int ssrc);
int Fox_DeleteLocalAudioStream();
int Fox_CreateRemoteAudioStream(unsigned int ssrc);
int Fox_DeleteRemoteAudioStream();

int Fox_CreateLocalVideoStream(int ssrc, void* view);
int Fox_DeleteLocalVideoStream();
int Fox_CreateRemoteVideoStream(int ssrc, void* view);
int Fox_DeleteRemoteVideoStream();

int Fox_InsertMediaData(char* data, int len);

