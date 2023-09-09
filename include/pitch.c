#include <bass/bass.h>
#include <aubio/aubio.h>

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define SAMPLE_RATE 48000.0

int main(int argc, char *argv[]) {
	/*
	int n_samples = 32*1024;
	int audioLength = n_samples;
	
	float audiodata[n_samples];
	double t;
	
	memset(audiodata,0,sizeof(audiodata));
	for(int i = 0; i < n_samples; i++) {
		t = (double)i/SAMPLE_RATE;
		audiodata[i] += sin((2.0*M_PI*550)*t);
	}
	*/

	if(HIWORD(BASS_GetVersion()) != BASSVERSION) 
	{
		printf("An incorrect version of BASS.DLL was loaded\n");
		return -1;
	}
	if(!BASS_Init(-1, 48000, BASS_DEVICE_MONO, NULL, NULL)) {
		printf("CANT INITIALIZE AUDIO DEVICE %d\n",BASS_ErrorGetCode());
		return -1;
	}
	
	HSTREAM str = BASS_StreamCreateFile(FALSE,"sample.wav",0,0,BASS_STREAM_DECODE|BASS_SAMPLE_FLOAT|BASS_SAMPLE_MONO);
	if(!str) {
		printf("Couldn't create file: %d\n",BASS_ErrorGetCode());
	}
	
	QWORD len = BASS_ChannelGetLength(str,BASS_POS_BYTE);
	if(len == -1) {
		printf("Couldn't get channel length: %d\n",BASS_ErrorGetCode());
	}
	
	int audioLength = len/sizeof(float);
	float audiodata[audioLength];
	DWORD res = BASS_ChannelGetData(str,audiodata,len);
	if(res == -1) {
		printf("Couldn't get data: %d\n",BASS_ErrorGetCode());
	}
	
	BASS_Free();

    uint_t bufferSize = 4096;  // Adjust according to your needs
	uint_t hopSize = bufferSize/4;  // Adjust according to your needs

    aubio_pitch_t *pitch = new_aubio_pitch("yinfast", bufferSize, hopSize, SAMPLE_RATE);
    fvec_t *inputBuffer = new_fvec(hopSize);

    // Configure the pitch detection
    aubio_pitch_set_unit(pitch, "Hz");  // Set the desired output unit

    // Process the audio in frames
    for (uint_t i = 0; i < audioLength; i += hopSize) {
        // Fill the input buffer with audio data
        for (uint_t j = 0; j < hopSize; j++) {
            if (i + j < audioLength) {
                inputBuffer->data[j] = audiodata[i + j];
            } else {
                inputBuffer->data[j] = 0.0;  // Pad with zeros if the audio data is not enough
            }
        }

        // Perform pitch detection on the input buffer
        fvec_t *outputBuffer = new_fvec(1);
        aubio_pitch_do(pitch, inputBuffer, outputBuffer);

        // Retrieve the pitch detection result
        float fundamentalFrequency = outputBuffer->data[0];
        printf("Fundamental Frequency: %.2f Hz\n", fundamentalFrequency);

        del_fvec(outputBuffer);
    }

    del_fvec(inputBuffer);
    del_aubio_pitch(pitch);
	
	return 0;
}

