/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

/*
 * Specifies the interface for the AEC core.
 */

#ifndef WEBRTC_MODULES_AUDIO_PROCESSING_AEC_AEC_CORE_H_
#define WEBRTC_MODULES_AUDIO_PROCESSING_AEC_AEC_CORE_H_

#ifndef AEC_OPTIMIZE
#define AEC_OPTIMIZE
#endif

#include <stddef.h>
#ifndef HOWL_DETECTED
#define HOWL_DETECTED
#endif
#include <memory>
#ifndef AEC_RECORD_DEBUG
//#define AEC_RECORD_DEBUG
#endif
extern "C" {
#include "webrtc/common_audio/ring_buffer.h"
}
#include "webrtc/base/constructormagic.h"
#include "webrtc/common_audio/wav_file.h"
#include "webrtc/modules/audio_processing/aec/aec_common.h"
#include "webrtc/modules/audio_processing/utility/block_mean_calculator.h"
#include "webrtc/typedefs.h"

namespace webrtc {

#define FRAME_LEN 80

#ifdef AEC_OPTIMIZE
#define FRAME_LEN_NEW 64
#endif
#ifdef AEC_OPTIMIZE
struct AecCoreNew;
#endif

#define PART_LEN 64               // Length of partition
#define PART_LEN1 (PART_LEN + 1)  // Unique fft coefficients
#define PART_LEN2 (PART_LEN * 2)  // Length of partition * 2
#define NUM_HIGH_BANDS_MAX 2      // Max number of high bands

class ApmDataDumper;

typedef float complex_t[2];
// For performance reasons, some arrays of complex numbers are replaced by twice
// as long arrays of float, all the real parts followed by all the imaginary
// ones (complex_t[SIZE] -> float[2][SIZE]). This allows SIMD optimizations and
// is better than two arrays (one for the real parts and one for the imaginary
// parts) as this other way would require two pointers instead of one and cause
// extra register spilling. This also allows the offsets to be calculated at
// compile time.

// Metrics
enum { kOffsetLevel = -100 };

typedef struct Stats {
  float instant;
  float average;
  float min;
  float max;
  float sum;
  float hisum;
  float himean;
  size_t counter;
  size_t hicounter;
} Stats;

// Number of partitions for the extended filter mode. The first one is an enum
// to be used in array declarations, as it represents the maximum filter length.
enum { kExtendedNumPartitions = 32 };
static const int kNormalNumPartitions = 12;

// Delay estimator constants, used for logging and delay compensation if
// if reported delays are disabled.
enum { kLookaheadBlocks = 15 };
enum {
  // 500 ms for 16 kHz which is equivalent with the limit of reported delays.
  kHistorySizeBlocks = 125
};

typedef struct PowerLevel {
  PowerLevel();

  BlockMeanCalculator framelevel;
  BlockMeanCalculator averagelevel;
  float minlevel;
} PowerLevel;

class BlockBuffer {
 public:
  BlockBuffer();
  ~BlockBuffer();
  void ReInit();
  void Insert(const float block[PART_LEN]);
  void ExtractExtendedBlock(float extended_block[PART_LEN]);
  int AdjustSize(int buffer_size_decrease);
  size_t Size();
  size_t AvaliableSpace();

 private:
  RingBuffer* buffer_;
};

class DivergentFilterFraction {
 public:
  DivergentFilterFraction();

  // Reset.
  void Reset();

  void AddObservation(const PowerLevel& nearlevel,
                      const PowerLevel& linoutlevel,
                      const PowerLevel& nlpoutlevel);

  // Return the latest fraction.
  float GetLatestFraction() const;

 private:
  // Clear all values added.
  void Clear();

  size_t count_;
  size_t occurrence_;
  float fraction_;

  RTC_DISALLOW_COPY_AND_ASSIGN(DivergentFilterFraction);
};

typedef struct CoherenceState {
  complex_t sde[PART_LEN1];  // cross-psd of nearend and error
  complex_t sxd[PART_LEN1];  // cross-psd of farend and nearend
  float sx[PART_LEN1], sd[PART_LEN1], se[PART_LEN1];  // far, near, error psd
} CoherenceState;

struct AecCore {
  explicit AecCore(int instance_index);
  ~AecCore();

  std::unique_ptr<ApmDataDumper> data_dumper;

  CoherenceState coherence_state;

  int farBufWritePos, farBufReadPos;

  int knownDelay;
  int inSamples, outSamples;
  int delayEstCtr;

  // Nearend buffer used for changing from FRAME_LEN to PART_LEN sample block
  // sizes. The buffer stores all the incoming bands and for each band a maximum
  // of PART_LEN - (FRAME_LEN - PART_LEN) values need to be buffered in order to
  // change the block size from FRAME_LEN to PART_LEN.
  float nearend_buffer[NUM_HIGH_BANDS_MAX + 1]
                      [PART_LEN - (FRAME_LEN - PART_LEN)];
  size_t nearend_buffer_size;
  float output_buffer[NUM_HIGH_BANDS_MAX + 1][2 * PART_LEN];
  size_t output_buffer_size;

  float eBuf[PART_LEN2];  // error

  float previous_nearend_block[NUM_HIGH_BANDS_MAX + 1][PART_LEN];

  float xPow[PART_LEN1];
  float dPow[PART_LEN1];
  float dMinPow[PART_LEN1];
  float dInitMinPow[PART_LEN1];
  float* noisePow;

  float xfBuf[2][kExtendedNumPartitions * PART_LEN1];  // farend fft buffer
  float wfBuf[2][kExtendedNumPartitions * PART_LEN1];  // filter fft
  // Farend windowed fft buffer.
  complex_t xfwBuf[kExtendedNumPartitions * PART_LEN1];

  float hNs[PART_LEN1];
  float hNlFbMin, hNlFbLocalMin;
  float hNlXdAvgMin;
  int hNlNewMin, hNlMinCtr;
  float overDrive;
  float overdrive_scaling;
  int nlp_mode;
  float outBuf[PART_LEN];
  int delayIdx;

  short stNearState, echoState;
  short divergeState;

  int xfBufBlockPos;

  BlockBuffer farend_block_buffer_;

  int system_delay;  // Current system delay buffered in AEC.

  int mult;  // sampling frequency multiple
  int sampFreq = 16000;
  size_t num_bands;
  uint32_t seed;

  float filter_step_size;  // stepsize
  float error_threshold;   // error threshold

  int noiseEstCtr;

  PowerLevel farlevel;
  PowerLevel nearlevel;
  PowerLevel linoutlevel;
  PowerLevel nlpoutlevel;

  int metricsMode;
  int stateCounter;
  Stats erl;
  Stats erle;
  Stats aNlp;
  Stats rerl;
  DivergentFilterFraction divergent_filter_fraction;

  // Quantities to control H band scaling for SWB input
  int freq_avg_ic;       // initial bin for averaging nlp gain
  int flag_Hband_cn;     // for comfort noise
  float cn_scale_Hband;  // scale for comfort noise in H band

  int delay_metrics_delivered;
  int delay_histogram[kHistorySizeBlocks];
  int num_delay_values;
  int delay_median;
  int delay_std;
  float fraction_poor_delays;
  int delay_logging_enabled;
  void* delay_estimator_farend;
  void* delay_estimator;
  // Variables associated with delay correction through signal based delay
  // estimation feedback.
  int previous_delay;
  int delay_correction_count;
  int shift_offset;
  float delay_quality_threshold;
  int frame_count;

  // 0 = delay agnostic mode (signal based delay correction) disabled.
  // Otherwise enabled.
  int delay_agnostic_enabled;
  // 1 = extended filter mode enabled, 0 = disabled.
  int extended_filter_enabled;
  // 1 = next generation aec mode enabled, 0 = disabled.
  int aec3_enabled;
  bool refined_adaptive_filter_enabled;

  // Runtime selection of number of filter partitions.
  int num_partitions;

  // Flag that extreme filter divergence has been detected by the Echo
  // Suppressor.
  int extreme_filter_divergence;
#ifdef AEC_RECORD_DEBUG
  FILE *near_in;
  FILE *far_in;
  FILE *aec_out;
#endif
#ifdef AEC_OPTIMIZE
  void* aecnewInst;
#endif
};

AecCore* WebRtcAec_CreateAec(int instance_count);  // Returns NULL on error.
void WebRtcAec_FreeAec(AecCore* aec);
int WebRtcAec_InitAec(AecCore* aec, int sampFreq);

#ifdef AEC_OPTIMIZE
struct AecCoreNew {
	explicit AecCoreNew(int instance_index);
	~AecCoreNew();

	std::unique_ptr<ApmDataDumper> data_dumper;

	CoherenceState coherence_state;

	int farBufWritePos, farBufReadPos;

	int knownDelay;
	int inSamples, outSamples;
	int delayEstCtr;

	// Nearend buffer used for changing from FRAME_LEN_NEW to PART_LEN sample block
	// sizes. The buffer stores all the incoming bands and for each band a maximum
	// of PART_LEN - (FRAME_LEN_NEW - PART_LEN) values need to be buffered in order to
	// change the block size from FRAME_LEN_NEW to PART_LEN.
	float nearend_buffer[NUM_HIGH_BANDS_MAX + 1]
		[PART_LEN - (FRAME_LEN_NEW - PART_LEN)];
	size_t nearend_buffer_size;
	float output_buffer[NUM_HIGH_BANDS_MAX + 1][2 * PART_LEN];
	size_t output_buffer_size;

	float eBuf[PART_LEN2];  // error

	float previous_nearend_block[NUM_HIGH_BANDS_MAX + 1][PART_LEN];

	float xPow[PART_LEN1];
	float dPow[PART_LEN1];
	float dMinPow[PART_LEN1];
	float dInitMinPow[PART_LEN1];
	float* noisePow;
#ifdef HOWL_DETECTED
	float SmoothHowlPow[PART_LEN1];
	float BandPow[16];
#endif

	float xfBuf[2][kExtendedNumPartitions * PART_LEN1];  // farend fft buffer
	float wfBuf[2][kExtendedNumPartitions * PART_LEN1];  // filter fft
														 // Farend windowed fft buffer.
	complex_t xfwBuf[kExtendedNumPartitions * PART_LEN1];

	float hNs[PART_LEN1];
	float hNlFbMin, hNlFbLocalMin;
	float hNlXdAvgMin;
	int hNlNewMin, hNlMinCtr;
	float overDrive;
	float overdrive_scaling;
	int nlp_mode;
	float outBuf[PART_LEN];
	int delayIdx;

	short stNearState, echoState;
	short divergeState;

	int xfBufBlockPos;

	BlockBuffer farend_block_buffer_;

	int system_delay;  // Current system delay buffered in AEC.

	int mult;  // sampling frequency multiple
	int sampFreq = 16000;
	size_t num_bands;
	uint32_t seed;

	float filter_step_size;  // stepsize
	float error_threshold;   // error threshold

	int noiseEstCtr;

	PowerLevel farlevel;
	PowerLevel nearlevel;
	PowerLevel linoutlevel;
	PowerLevel nlpoutlevel;

	int metricsMode;
	int stateCounter;
	Stats erl;
	Stats erle;
	Stats aNlp;
	Stats rerl;
	DivergentFilterFraction divergent_filter_fraction;

	// Quantities to control H band scaling for SWB input
	int freq_avg_ic;       // initial bin for averaging nlp gain
	int flag_Hband_cn;     // for comfort noise
	float cn_scale_Hband;  // scale for comfort noise in H band

	int delay_metrics_delivered;
	int delay_histogram[kHistorySizeBlocks];
	int num_delay_values;
	int delay_median;
	int delay_std;
	float fraction_poor_delays;
	int delay_logging_enabled;
	void* delay_estimator_farend;
	void* delay_estimator;
	// Variables associated with delay correction through signal based delay
	// estimation feedback.
	int previous_delay;
	int delay_correction_count;
	int shift_offset;
	float delay_quality_threshold;
	int frame_count;

	// 0 = delay agnostic mode (signal based delay correction) disabled.
	// Otherwise enabled.
	int delay_agnostic_enabled;
	// 1 = extended filter mode enabled, 0 = disabled.
	int extended_filter_enabled;
	// 1 = next generation aec mode enabled, 0 = disabled.
	int aec3_enabled;
	bool refined_adaptive_filter_enabled;

	// Runtime selection of number of filter partitions.
	int num_partitions;
	// Flag that extreme filter divergence has been detected by the Echo
	// Suppressor.
	int extreme_filter_divergence;
};

AecCoreNew* WebRtcAec_CreateAec_New(int instance_count);  // Returns NULL on error.
void WebRtcAec_FreeAec_New(AecCoreNew* aec);
int WebRtcAec_InitAec_New(AecCoreNew* aec, int sampFreq);
#endif


void WebRtcAec_InitAec_SSE2(void);
#if defined(MIPS_FPU_LE)
void WebRtcAec_InitAec_mips(void);
#endif
#if defined(WEBRTC_HAS_NEON)
void WebRtcAec_InitAec_neon(void);
#endif

void WebRtcAec_BufferFarendBlock(AecCore* aec, const float* farend);
void WebRtcAec_ProcessFrames(AecCore* aec,
                             const float* const* nearend,
                             size_t num_bands,
                             size_t num_samples,
                             int knownDelay,
                             float* const* out);

// A helper function to call adjust the farend buffer size.
// Returns the number of elements the size was decreased with, and adjusts
// |system_delay| by the corresponding amount in ms.
int WebRtcAec_AdjustFarendBufferSizeAndSystemDelay(AecCore* aec,
                                                   int size_decrease);

// Calculates the median, standard deviation and amount of poor values among the
// delay estimates aggregated up to the first call to the function. After that
// first call the metrics are aggregated and updated every second. With poor
// values we mean values that most likely will cause the AEC to perform poorly.
// TODO(bjornv): Consider changing tests and tools to handle constant
// constant aggregation window throughout the session instead.
int WebRtcAec_GetDelayMetricsCore(AecCore* self,
                                  int* median,
                                  int* std,
                                  float* fraction_poor_delays);

// Returns the echo state (1: echo, 0: no echo).
int WebRtcAec_echo_state(AecCore* self);

// Gets statistics of the echo metrics ERL, ERLE, A_NLP.
void WebRtcAec_GetEchoStats(AecCore* self,
                            Stats* erl,
                            Stats* erle,
                            Stats* a_nlp,
                            float* divergent_filter_fraction);

// Sets local configuration modes.
void WebRtcAec_SetConfigCore(AecCore* self,
                             int nlp_mode,
                             int metrics_mode,
                             int delay_logging);

// Non-zero enables, zero disables.
void WebRtcAec_enable_delay_agnostic(AecCore* self, int enable);

// Returns non-zero if delay agnostic (i.e., signal based delay estimation) is
// enabled and zero if disabled.
int WebRtcAec_delay_agnostic_enabled(AecCore* self);

// Non-zero enables, zero disables.
void WebRtcAec_enable_aec3(AecCore* self, int enable);

// Returns 1 if the next generation aec is enabled and zero if disabled.
int WebRtcAec_aec3_enabled(AecCore* self);

// Turns on/off the refined adaptive filter feature.
void WebRtcAec_enable_refined_adaptive_filter(AecCore* self, bool enable);

// Returns whether the refined adaptive filter is enabled.
bool WebRtcAec_refined_adaptive_filter(const AecCore* self);

// Enables or disables extended filter mode. Non-zero enables, zero disables.
void WebRtcAec_enable_extended_filter(AecCore* self, int enable);

// Returns non-zero if extended filter mode is enabled and zero if disabled.
int WebRtcAec_extended_filter_enabled(AecCore* self);

// Returns the current |system_delay|, i.e., the buffered difference between
// far-end and near-end.
int WebRtcAec_system_delay(AecCore* self);

// Sets the |system_delay| to |value|.  Note that if the value is changed
// improperly, there can be a performance regression.  So it should be used with
// care.
void WebRtcAec_SetSystemDelay(AecCore* self, int delay);


#ifdef AEC_OPTIMIZE
void WebRtcAec_BufferFarendBlock_New(AecCoreNew* aec, const float* farend);
void WebRtcAec_ProcessFrames_New(AecCoreNew* aec,
	const float* const* nearend,
	size_t num_bands,
	size_t num_samples,
	int knownDelay,
	float* const* out);

// A helper function to call adjust the farend buffer size.
// Returns the number of elements the size was decreased with, and adjusts
// |system_delay| by the corresponding amount in ms.
int WebRtcAec_AdjustFarendBufferSizeAndSystemDelay_New(AecCoreNew* aec,
	int size_decrease);

// Calculates the median, standard deviation and amount of poor values among the
// delay estimates aggregated up to the first call to the function. After that
// first call the metrics are aggregated and updated every second. With poor
// values we mean values that most likely will cause the AEC to perform poorly.
// TODO(bjornv): Consider changing tests and tools to handle constant
// constant aggregation window throughout the session instead.
int WebRtcAec_GetDelayMetricsCore_New(AecCoreNew* self,
	int* median,
	int* std,
	float* fraction_poor_delays);

// Returns the echo state (1: echo, 0: no echo).
int WebRtcAec_echo_state_New(AecCoreNew* self);

// Gets statistics of the echo metrics ERL, ERLE, A_NLP.
void WebRtcAec_GetEchoStats_New(AecCoreNew* self,
	Stats* erl,
	Stats* erle,
	Stats* a_nlp,
	float* divergent_filter_fraction);

// Sets local configuration modes.
void WebRtcAec_SetConfigCore_New(AecCoreNew* self,
	int nlp_mode,
	int metrics_mode,
	int delay_logging);

// Non-zero enables, zero disables.
void WebRtcAec_enable_delay_agnostic_New(AecCoreNew* self, int enable);

// Returns non-zero if delay agnostic (i.e., signal based delay estimation) is
// enabled and zero if disabled.
int WebRtcAec_delay_agnostic_enabled_New(AecCoreNew* self);

// Non-zero enables, zero disables.
void WebRtcAec_enable_aec3_New(AecCoreNew* self, int enable);

// Returns 1 if the next generation aec is enabled and zero if disabled.
int WebRtcAec_aec3_enabled_New(AecCoreNew* self);

// Turns on/off the refined adaptive filter feature.
void WebRtcAec_enable_refined_adaptive_filter_New(AecCoreNew* self, bool enable);

// Returns whether the refined adaptive filter is enabled.
bool WebRtcAec_refined_adaptive_filter_New(const AecCoreNew* self);

// Enables or disables extended filter mode. Non-zero enables, zero disables.
void WebRtcAec_enable_extended_filter_New(AecCoreNew* self, int enable);

// Returns non-zero if extended filter mode is enabled and zero if disabled.
int WebRtcAec_extended_filter_enabled_New(AecCoreNew* self);

// Returns the current |system_delay|, i.e., the buffered difference between
// far-end and near-end.
int WebRtcAec_system_delay_New(AecCoreNew* self);

// Sets the |system_delay| to |value|.  Note that if the value is changed
// improperly, there can be a performance regression.  So it should be used with
// care.
void WebRtcAec_SetSystemDelay_New(AecCoreNew* self, int delay);
#endif

#ifdef AEC_OPTIMIZE
#define MAX_RESAMP_LEN_NEW (5 * FRAME_LEN_NEW)

// Errors
#define AEC_UNSPECIFIED_ERROR 12000
#define AEC_UNSUPPORTED_FUNCTION_ERROR 12001
#define AEC_UNINITIALIZED_ERROR 12002
#define AEC_NULL_POINTER_ERROR 12003
#define AEC_BAD_PARAMETER_ERROR 12004

// Warnings
#define AEC_BAD_PARAMETER_WARNING 12050

enum { kAecNlpConservativenew = 0, kAecNlpModeratenew, kAecNlpAggressivenew };

enum { kAecFalsenew = 0, kAecTruenew };

typedef struct {
	int16_t nlpMode;      // default kAecNlpModerate
	int16_t skewMode;     // default kAecFalse
	int16_t metricsMode;  // default kAecFalse
	int delay_logging;    // default kAecFalse
						  // float realSkew;
} AecConfigNew;
#endif

#ifdef AEC_OPTIMIZE
typedef struct AecNew {
	AecNew();
	~AecNew();

	std::unique_ptr<ApmDataDumper> data_dumper;

	int delayCtr;
	int sampFreq;
	int splitSampFreq;
	int scSampFreq;
	float sampFactor;  // scSampRate / sampFreq
	short skewMode;
	int bufSizeStart;
	int knownDelay;
	int rate_factor;

	short initFlag;  // indicates if AEC has been initialized

					 // Variables used for averaging far end buffer size
	short counter;
	int sum;
	short firstVal;
	short checkBufSizeCtr;

	// Variables used for delay shifts
	short msInSndCardBuf;
	short filtDelay;  // Filtered delay estimate.
	int timeForDelayChange;
	int startup_phase;
	int checkBuffSize;
	short lastDelayDiff;

	// Structures
	//void* resampler;

	int skewFrCtr;
	int resample;  // if the skew is small enough we don't resample
	int highSkewCtr;
	float skew;

	RingBuffer* far_pre_buf;  // Time domain far-end pre-buffer.

	int farend_started;

	// AecNew instance counter.
	static int instance_count;
	AecCoreNew* aec;
} AecNew;

/*
* Allocates the memory needed by the AEC. The memory needs to be initialized
* separately using the WebRtcAec_Init_New() function. Returns a pointer to the
* object or NULL on error.
*/
void* WebRtcAec_Create_New();

/*
* This function releases the memory allocated by WebRtcAec_Create_New().
*
* Inputs                       Description
* -------------------------------------------------------------------
* void*        aecInst         Pointer to the AEC instance
*/
void WebRtcAec_Free_New(void* aecInst);

/*
* Initializes an AEC instance.
*
* Inputs                       Description
* -------------------------------------------------------------------
* void*          aecInst       Pointer to the AEC instance
* int32_t        sampFreq      Sampling frequency of data
* int32_t        scSampFreq    Soundcard sampling frequency
*
* Outputs                      Description
* -------------------------------------------------------------------
* int32_t        return        0: OK
*                             -1: error
*/
int32_t WebRtcAec_Init_New(void* aecInst, int32_t sampFreq, int32_t scSampFreq);

/*
* Inserts an 80 or 160 sample block of data into the farend buffer.
*
* Inputs                       Description
* -------------------------------------------------------------------
* void*          aecInst       Pointer to the AEC instance
* const float*   farend        In buffer containing one frame of
*                              farend signal for L band
* int16_t        nrOfSamples   Number of samples in farend buffer
*
* Outputs                      Description
* -------------------------------------------------------------------
* int32_t        return        0: OK
*                              12000-12050: error code
*/
int32_t WebRtcAec_BufferFarend_New(void* aecInst,
	const float* farend,
	size_t nrOfSamples);

/*
* Reports any errors that would arise if buffering a farend buffer
*
* Inputs                       Description
* -------------------------------------------------------------------
* void*          aecInst       Pointer to the AEC instance
* const float*   farend        In buffer containing one frame of
*                              farend signal for L band
* int16_t        nrOfSamples   Number of samples in farend buffer
*
* Outputs                      Description
* -------------------------------------------------------------------
* int32_t        return        0: OK
*                              12000-12050: error code
*/
int32_t WebRtcAec_GetBufferFarendError_New(void* aecInst,
	const float* farend,
	size_t nrOfSamples);

/*
* Runs the echo canceller on an 80 or 160 sample blocks of data.
*
* Inputs                       Description
* -------------------------------------------------------------------
* void*         aecInst        Pointer to the AEC instance
* float* const* nearend        In buffer containing one frame of
*                              nearend+echo signal for each band
* int           num_bands      Number of bands in nearend buffer
* int16_t       nrOfSamples    Number of samples in nearend buffer
* int16_t       msInSndCardBuf Delay estimate for sound card and
*                              system buffers
* int16_t       skew           Difference between number of samples played
*                              and recorded at the soundcard (for clock skew
*                              compensation)
*
* Outputs                      Description
* -------------------------------------------------------------------
* float* const* out            Out buffer, one frame of processed nearend
*                              for each band
* int32_t       return         0: OK
*                              12000-12050: error code
*/
int32_t WebRtcAec_Process_New(void* aecInst,
	const float* const* nearend,
	size_t num_bands,
	float* const* out,
	size_t nrOfSamples,
	int16_t msInSndCardBuf,
	int32_t skew);

/*
* This function enables the user to set certain parameters on-the-fly.
*
* Inputs                       Description
* -------------------------------------------------------------------
* void*          handle        Pointer to the AEC instance
* AecConfig      config        Config instance that contains all
*                              properties to be set
*
* Outputs                      Description
* -------------------------------------------------------------------
* int            return        0: OK
*                              12000-12050: error code
*/
int WebRtcAec_set_config_New(void* handle, AecConfigNew config);

/*
* Gets the current echo status of the nearend signal.
*
* Inputs                       Description
* -------------------------------------------------------------------
* void*          handle        Pointer to the AEC instance
*
* Outputs                      Description
* -------------------------------------------------------------------
* int*           status        0: Almost certainly nearend single-talk
*                              1: Might not be neared single-talk
* int            return        0: OK
*                              12000-12050: error code
*/
int WebRtcAec_get_echo_status_New(void* handle, int* status);

/*
* Gets the current echo metrics for the session.
*
* Inputs                       Description
* -------------------------------------------------------------------
* void*          handle        Pointer to the AEC instance
*
* Outputs                      Description
* -------------------------------------------------------------------
* AecMetrics*    metrics       Struct which will be filled out with the
*                              current echo metrics.
* int            return        0: OK
*                              12000-12050: error code
*/

//int WebRtcAec_GetMetrics_New(void* handle, AecMetrics* metrics);

/*
* Gets the current delay metrics for the session.
*
* Inputs                       Description
* -------------------------------------------------------------------
* void*   handle               Pointer to the AEC instance
*
* Outputs                      Description
* -------------------------------------------------------------------
* int*    median               Delay median value.
* int*    std                  Delay standard deviation.
* float*  fraction_poor_delays Fraction of the delay estimates that may
*                              cause the AEC to perform poorly.
*
* int            return        0: OK
*                              12000-12050: error code
*/
int WebRtcAec_GetDelayMetrics_New(void* handle,
	int* median,
	int* std,
	float* fraction_poor_delays);

// Returns a pointer to the low level AEC handle.
//
// Input:
//  - handle                    : Pointer to the AEC instance.
//
// Return value:
//  - AecCoreNew pointer           : NULL for error.
//
struct AecCoreNew* WebRtcAec_aec_core_New(void* handle);
#endif

}  // namespace webrtc

#endif  // WEBRTC_MODULES_AUDIO_PROCESSING_AEC_AEC_CORE_H_
