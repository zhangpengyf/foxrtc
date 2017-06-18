/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.6
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.foxrtc.android;

public class Foxrtc_androidJNI {
  public final static native long new_FoxrtcTransport();
  public final static native void delete_FoxrtcTransport(long jarg1);
  public final static native int FoxrtcTransport_SendRtp(long jarg1, FoxrtcTransport jarg1_, String jarg2, int jarg3);
  public final static native int FoxrtcTransport_SendRtcp(long jarg1, FoxrtcTransport jarg1_, String jarg2, int jarg3);
  public final static native void delete_Foxrtc(long jarg1);
  public final static native long Foxrtc_Instance();
  public final static native int Foxrtc_Init(long jarg1, Foxrtc jarg1_, long jarg2, FoxrtcTransport jarg2_);
  public final static native int Foxrtc_Uninit(long jarg1, Foxrtc jarg1_);
  public final static native int Foxrtc_GetDeviceInfo(long jarg1, Foxrtc jarg1_);
  public final static native int Foxrtc_OpenCamera(long jarg1, Foxrtc jarg1_, int jarg2);
  public final static native int Foxrtc_CloseCamera(long jarg1, Foxrtc jarg1_);
  public final static native int Foxrtc_CreateLocalAudioStream(long jarg1, Foxrtc jarg1_, long jarg2);
  public final static native int Foxrtc_DeleteLocalAudioStream(long jarg1, Foxrtc jarg1_);
  public final static native int Foxrtc_CreateRemoteAudioStream(long jarg1, Foxrtc jarg1_, long jarg2);
  public final static native int Foxrtc_DeleteRemoteAudioStream(long jarg1, Foxrtc jarg1_);
  public final static native int Foxrtc_CreateLocalVideoStream(long jarg1, Foxrtc jarg1_, int jarg2, Object jarg3);
  public final static native int Foxrtc_DeleteLocalVideoStream(long jarg1, Foxrtc jarg1_);
  public final static native int Foxrtc_CreateRemoteVideoStream(long jarg1, Foxrtc jarg1_, int jarg2, Object jarg3);
  public final static native int Foxrtc_DeleteRemoteVideoStream(long jarg1, Foxrtc jarg1_);
  public final static native int Foxrtc_IncomingData(long jarg1, Foxrtc jarg1_, String jarg2, int jarg3);
}