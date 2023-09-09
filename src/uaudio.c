#include <uaudio.h>
#include <usystem.h>
#include <aubio/aubio.h>
#include <bass/bass.h>
#include <bassasio/bassasio.h>
#include <bass_fx/bass_fx.h>

#include <math.h>
#include <stdio.h>

struct Frequency {
	float freq;
	char name[4];
};

audio_struct* audio_;
aubio_pitch_t* pitch_ = NULL;
fvec_t* inputBuffer;

float master_vol,gain_pos,gain_neg,clip_pos,clip_neg,feedback,tune_freq;
float eq[EQ_BANDS],*samples;
struct bandpass_struct bp[EQ_BANDS];
int eq_active, type_pos, type_neg, tune_mode, bufferSize, hopSize, rmutex, sample_size;

const char* eq_strings[] = {
	"31.25","62.5","125","250","500","1K","2K","4K","8K","16K"
};

const char* clip_strings[] = {
	"OFF","HARD","SOFT"
};

struct Frequency frequencies[] = {
        {32.703, "C1"},
        {34.648, "C#1"},
        {36.708, "D1"},
        {38.891, "D#1"},
        {41.203, "E1"},
        {43.654, "F1"},
        {46.249, "F#1"},
        {48.999, "G1"},
        {51.913, "G#1"},
        {55.000, "A1"},
        {58.270, "A#1"},
        {61.735, "B1"},
        {65.406, "C2"},
        {69.296, "C#2"},
        {73.416, "D2"},
        {77.782, "D#2"},
        {82.407, "E2"},
        {87.307, "F2"},
        {92.499, "F#2"},
        {97.999, "G2"},
        {103.83, "G#2"},
        {110.00, "A2"},
        {116.54, "A#2"},
        {123.47, "B2"},
        {130.81, "C3"},
        {138.59, "C#3"},
        {146.83, "D3"},
        {155.56, "D#3"},
        {164.81, "E3"},
        {174.61, "F3"},
        {185.00, "F#3"},
        {196.00, "G3"},
        {207.65, "G#3"},
        {220.00, "A3"},
        {233.08, "A#3"},
        {246.94, "B3"},
        {261.63, "C4"},
        {277.18, "C#4"},
        {293.66, "D4"},
        {311.13, "D#4"},
        {329.63, "E4"},
        {349.23, "F4"},
        {369.99, "F#4"},
        {392.00, "G4"},
        {415.30, "G#4"},
        {440.00, "A4"},
        {466.16, "A#4"},
        {493.88, "B4"},
        {523.25, "C5"},
        {554.37, "C#5"},
        {587.33, "D5"},
        {622.25, "D#5"},
        {659.26, "E5"},
        {698.46, "F5"},
        {739.99, "F#5"},
        {783.99, "G5"},
        {830.61, "G#5"},
        {880.00, "A5"},
        {932.33, "A#5"},
        {987.77, "B5"},
        {1046.5, "C6"},
        {1108.7, "C#6"},
        {1174.7, "D6"},
        {1244.5, "D#6"},
        {1318.5, "E6"},
        {1396.9, "F6"},
        {1479.9, "F#6"},
        {1567.9, "G6"},
        {1661.2, "G#6"},
        {1760.0, "A6"},
        {1864.7, "A#6"},
        {1975.5, "B6"},
        {2093.0, "C7"},
        {2217.5, "C#7"},
        {2349.3, "D7"},
        {2489.0, "D#7"},
        {2637.0, "E7"},
        {2793.8, "F7"},
        {2959.9, "F#7"},
        {3135.9, "G7"},
        {3322.4, "G#7"},
        {3520.0, "A7"},
        {3729.3, "A#7"},
        {3951.1, "B7"},
        {4186.0, "C8"}
    };
	
static char 	errbuf[1024];
static HSTREAM 	outchan;
static HDSP		distortion,equalizer,recorder;

static int initialize_bass();
static int initialize_bassfx();
static int initialize_asio();
static int prepare_playback();

// display error messages
static void Error(const char *es) {
	sprintf(errbuf, "%s\n\3ERROR CODE:\1 %ld/%d", es, BASS_ASIO_ErrorGetCode(), BASS_ErrorGetCode());
	post_message(errbuf);
}

DWORD CALLBACK PlaybackProc(BOOL isinput, DWORD channel, void *buffer, DWORD length, void *user) {
	if (isinput) { // recording
		BASS_StreamPutData(outchan,buffer,length);
		return 0;
	} 
	else { // playing
		int c = BASS_ChannelGetData(outchan,buffer,length); // get data from the decoder
		if (c == -1) c = 0;
		return c;
	}
}

void CALLBACK DISTProc(HDSP handle, DWORD channel, void* buffer, DWORD length, void* user) {
	if(type_pos == OFF && type_neg == OFF)
		return;
	
	float* s = (float*)buffer;
    for (; length; length -= 4, s++) {
		if(s[0] > 0) {
			switch(type_pos) {
				case HARD:
					s[0] *= gain_pos;
					if(s[0] > clip_pos) {
						s[0] = clip_pos;
					}
				break;
				case SOFT:
					s[0] *= gain_pos;
					if(s[0] > clip_pos) {
						s[0] = clip_pos + sqrtf((s[0]-clip_pos)/(MAX_GAIN-clip_pos))*clip_pos/2;
					}
				break;
				case OFF:
					continue;
				break;
			}
		} else {
			switch(type_neg) {
				case HARD:
					s[0] *= gain_neg;
					if(s[0] < -clip_neg) {
						s[0] = -clip_neg;
					}
				break;
				case SOFT:
					s[0] *= gain_neg;
					if(s[0] < -clip_neg) {
						s[0] = -clip_neg - sqrtf((-s[0]-clip_neg)/(MAX_GAIN-clip_neg))*clip_neg/2;
					}
				break;
				case OFF:
					continue;
				break;
			}
		}
    }
}

void CALLBACK EQProc(HDSP handle, DWORD channel, void* buffer, DWORD length, void* user) {
	if(!eq_active)
		return;
	
	float* s = (float*)buffer;
	float v;
	int i;
    for (; length; length -= 4, s++) {
		v = 0;
		for (i = 0; i < EQ_BANDS; i++) {
			bp[i].w0 = bp[i].d1*bp[i].w1+bp[i].d2*bp[i].w2+bp[i].d3*bp[i].w3+bp[i].d4*bp[i].w4+s[0];
			v += (bp[i].A*(bp[i].w0-2.0*bp[i].w2+bp[i].w4))*eq[i];
			bp[i].w4 = bp[i].w3;
			bp[i].w3 = bp[i].w2;
			bp[i].w2 = bp[i].w1;
			bp[i].w1 = bp[i].w0;
		}
		s[0] = v*master_vol;
	}
}

void CALLBACK RecordProc(HDSP handle, DWORD channel, void* buffer, DWORD length, void* user) {
	if(!rmutex)
		return;
	
	int len = length/4;
	
	if (sample_size + len <= bufferSize) {
		memcpy(samples+sample_size,buffer,length);
		sample_size += len;
	} else {
		memcpy(samples+sample_size,buffer,(bufferSize-sample_size)*4);
		sample_size = bufferSize;
	}
	
	if (sample_size == bufferSize)
		rmutex = 0;
}

int	init_audio() {
	if(!audio_) {
		audio_ = malloc(sizeof(audio_struct));
		memset(audio_,0,sizeof(audio_struct));
	}
	
	if(!pitch_) {
		tune_freq = -1;
		bufferSize = 4096;
		hopSize = bufferSize/4;
		pitch_ = new_aubio_pitch("yinfast", bufferSize, hopSize, 48000);
		inputBuffer = new_fvec(hopSize);
		aubio_pitch_set_unit(pitch_, "Hz");
		
		samples = (float*)malloc(sizeof(float)*bufferSize);
	}
	
	if(!initialize_bass() || !initialize_bassfx() || !initialize_asio()) {
		return 0;
	}
	
	return 1;
}

void start_playback() {
	if(!audio_->bass_status) {
		post_message("OUTPUT DEVICE NOT INITIALIZED. ATTEMPTING TO INITIALIZE...");
		if(!initialize_bass())
			return;
	}
	if(!audio_->bassfx_status) {
		post_message("BASSFX NOT INITIALIZED. ATTEMPTING TO INITIALIZE...");
		if(!initialize_bassfx())
			return;
	}
	if(!audio_->asio_status) {
		post_message("ASIO DEVICE NOT INITIALIZED. ATTEMPTING TO INITIALIZE...");
		if(!initialize_asio())
			return;
	}
	
	if(!prepare_playback()) {
		BASS_ASIO_Free();
		audio_->asio_status = 0;
	}
	
	if (!BASS_ASIO_Start(0,0)) {
		Error("\2ERROR:\1 FAILED TO START ASIO DEVICE");
		return;
	}
	
	post_message("DEVICE SUCCESSFULLY STARTED.\nLISTENING...");
	audio_->active = 1;
}

void stop_devices() {
	if(BASS_ASIO_IsStarted()) {
		if (!BASS_ASIO_Stop()) {
			Error("\3WARNING:\1 FAILED TO STOP DEVICE");
		} else {
			post_message("DEVICE STOPPED");
		}
	} else {
		post_message("NO DEVICE IS STARTED");
	}
	audio_->active = 0;
	tune_mode = 0;
}

void toggle_tuner() {
	if(!audio_ || !audio_->active) {
		tune_mode = 0;
		return;
	}
	
	tune_mode = !tune_mode;
}

void tune() {
	if (rmutex) {
		return;
	}
	
	float freq = 0;
	int count = 0;
	for (int i = 0; i < bufferSize; i += hopSize) {
        // Fill the input buffer with audio data
        for (int j = 0; j < hopSize; j++) {
            if (i + j < bufferSize) {
                inputBuffer->data[j] = samples[i + j];
            } else {
                inputBuffer->data[j] = 0.0;  // Pad with zeros if the audio data is not enough
            }
        }

        // Perform pitch detection on the input buffer
        fvec_t* outputBuffer = new_fvec(1);
        aubio_pitch_do(pitch_, inputBuffer, outputBuffer);

        // Retrieve the pitch detection result
        freq += outputBuffer->data[0];
		count++;
        del_fvec(outputBuffer);
    }
	
	tune_freq = freq/count;
	sample_size = 0;
	rmutex = 1;
	
	//Find and print frequency name
	float min = 9999;
	int mindex = -1;
	for (int i = 0; i < sizeof(frequencies)/sizeof(struct Frequency); i++) {
		if (fabs(tune_freq-frequencies[i].freq) < min) {
			min = fabs(tune_freq-frequencies[i].freq);
			mindex = i;
		}
	}
	printf("%s\r", frequencies[mindex].name);
}

void destroy_audio() {
	stop_devices();
	if(audio_) {
		free(audio_);
	}
	
	if(pitch_) {
		del_fvec(inputBuffer);
		del_aubio_pitch(pitch_);
		free(samples);
	}
	
	BASS_ASIO_Free();
	BASS_Free();
}

static int initialize_bass() {
	if(!audio_->bass_status) {
		// check the correct BASS was loaded
		if (HIWORD(BASS_GetVersion()) != BASSVERSION) {
			fprintf(stderr,"An incorrect version of BASS.DLL was loaded");
			return 0;
		}
		
		// initialize bass
		if(!BASS_Init(0, 48000, 0, 0, 0)) {
			Error("\2ERROR\1: CANT INITIALIZE AUDIO DEVICE");
			return 0;
		}
		
		if(!outchan) {
			outchan = BASS_StreamCreate(48000, 1, BASS_SAMPLE_FLOAT|BASS_STREAM_DECODE, STREAMPROC_PUSH, NULL);
			if (!outchan) {
				Error("\2ERROR:\1 FAILED TO CREATE AUDIO OUTPUT STREAM");
				return 0;
			}
		}
		
		if(!(distortion = BASS_ChannelSetDSP(outchan,DISTProc,NULL,5))) {
			Error("\2ERROR:\1 UNABLE TO SET DISTORTION DSP");
			return 0;
		}
		
		if(!(equalizer = BASS_ChannelSetDSP(outchan,EQProc,NULL,4))) {
			Error("\2ERROR:\1 UNABLE TO SET EQUALIZER DSP");
			return 0;
		}
		
		if(!(recorder = BASS_ChannelSetDSP(outchan,RecordProc,NULL,3))) {
			Error("\2ERROR:\1 UNABLE TO SET TUNING DSP");
			return 0;
		}
	}
	
	audio_->bass_status = 1;
	
	return 1;
}

static int initialize_bassfx() {
	// check the correct BASS_FX was loaded
	if (HIWORD(BASS_FX_GetVersion()) != BASSVERSION) {
		fprintf(stderr,"An incorrect version of BASS_FX.DLL was loaded");
		return 0;
	}
	
	return 1;
}

static int initialize_asio() {
	if(!audio_->asio_status) {
		// check the correct BASSASIO was loaded
		if (HIWORD(BASS_ASIO_GetVersion()) != BASSASIOVERSION) {
			fprintf(stderr,"An incorrect version of BASSASSIO.DLL was loaded");
			return 0;
		}
		
		// initialize first available ASIO device
		if (!BASS_ASIO_Init(-1, BASS_ASIO_THREAD)) {
			Error("\3WARNING:\1 CANT FIND ASIO DEVICE");
			return 0;
		}
	}
	
	audio_->asio_status = 1;
	
	return 1;
}

static int prepare_playback() {
	if (BASS_ASIO_IsStarted()) {
		BASS_ASIO_Stop();
	}
	
	if (!BASS_ASIO_ChannelReset(TRUE, -1, BASS_ASIO_RESET_ENABLE)) {
		Error("\2ERROR:\1 CANT RESET INPUTS");
		return 0;
	}
	
	if (!BASS_ASIO_ChannelReset(FALSE, -1, BASS_ASIO_RESET_ENABLE)) {
		Error("\2ERROR:\1 CANT RESET OUTPUTS");
		return 0;
	}
	
	if (!BASS_ASIO_ChannelEnable(TRUE, 1, PlaybackProc, NULL)) {
		Error("\2ERROR:\1 CANT ENABLE INPUT 1");
		return 0;
	}
	
	if (!BASS_ASIO_ChannelEnable(FALSE, 0, PlaybackProc, 0)) {
		Error("\2ERROR:\1 CANT ENABLE OUTPUT 1");
		return 0;
	}
	
	if (!BASS_ASIO_ChannelSetFormat(TRUE, 1, BASS_ASIO_FORMAT_FLOAT)) {
		Error("\2ERROR:\1 CANT SET INPUT FORMAT");
		return 0;
	}
	
	if (!BASS_ASIO_ChannelSetFormat(FALSE, 0, BASS_ASIO_FORMAT_FLOAT)) {
		Error("\2ERROR:\1 CANT SET OUTPUT FORMAT");
		return 0;
	}
	
	if (!BASS_ASIO_ChannelEnableMirror(1, FALSE, 0)) {
		Error("\2ERROR:\1 CANT MIRROR OUTPUT CHANNELS");
		return 0;
	}
	
	return 1;
}