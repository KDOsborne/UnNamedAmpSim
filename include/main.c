#include <windows.h>
#include <aubio/aubio.h>
#include <bass/bass.h>
#include <bassasio/bassasio.h>
#include <stdlib.h>
#include <stdio.h>

HSTREAM chan;

// display error messages
void Error(const char *es) {
	fprintf(stderr, "%s\n(error code: %ld/%d)\n", es, BASS_ASIO_ErrorGetCode(), BASS_ErrorGetCode());
}

DWORD CALLBACK ASIOProc(BOOL isinput, DWORD channel, void *buffer, DWORD length, void *user)
{
	if (isinput) { // recording
		BASS_StreamPutData(chan,buffer,length);
		return 0;
	} 
	else { // playing
		int c = BASS_ChannelGetData(chan,buffer,length); // get data from the decoder
		if (c == -1) c = 0;
		return c;
	}
}

int main(int argc, char* argv[])
{
	chan = 0;
	
	// check the correct BASS was loaded
	if (HIWORD(BASS_GetVersion()) != BASSVERSION) {
		fprintf(stderr,"An incorrect version of BASS.DLL was loaded");
		return 1;
	}
	
	// check the correct BASSASIO was loaded
	if (HIWORD(BASS_ASIO_GetVersion()) != BASSASIOVERSION) {
		fprintf(stderr,"An incorrect version of BASSASSIO.DLL was loaded");
		return 1;
	}
	
	BASS_ASIO_DEVICEINFO di;
	int a;
	for (a = 0; BASS_ASIO_GetDeviceInfo(a, &di); a++) {
		printf("dev %d: %s\ndriver: %s\n", a, di.name, di.driver);
		if (!BASS_ASIO_Init(a, BASS_ASIO_THREAD)) {
			printf("\tunavailable\n");
			continue;
		}
		printf("\trate: %g\n", BASS_ASIO_GetRate());
		{
			BASS_ASIO_CHANNELINFO i;
			int b;
			for (b = 0; BASS_ASIO_ChannelGetInfo(TRUE, b, &i); b++)
				printf("\tin %d: %s (group %ld, format %ld)\n", b, i.name, i.group, i.format);
			for (b = 0; BASS_ASIO_ChannelGetInfo(FALSE, b, &i); b++)
				printf("\tout %d: %s (group %ld, format %ld)\n", b, i.name, i.group, i.format);
			if (i.format < BASS_ASIO_FORMAT_DSD_LSB && BASS_ASIO_SetDSD(TRUE)) {
				printf("\tDSD:\n");
				for (b = 0; BASS_ASIO_ChannelGetInfo(TRUE, b, &i); b++)
					printf("\tin %d: %s (group %ld, format %ld)\n", b, i.name, i.group, i.format);
				for (b = 0; BASS_ASIO_ChannelGetInfo(FALSE, b, &i); b++)
					printf("\tout %d: %s (group %ld, format %ld)\n", b, i.name, i.group, i.format);
			}
		}
		BASS_ASIO_Free();
	}
	
	// initialize bass
	if(!BASS_Init(0, 48000, 0, 0, 0)) {
		Error("Can't initialize audio device");
		return 1;
	}
	
	// initialize first available ASIO device
	if (!BASS_ASIO_Init(-1, BASS_ASIO_THREAD)) {
		Error("Can't find initialize ASIO device");
		return 1;
	}
	
	if (!BASS_ASIO_ChannelReset(TRUE, -1, BASS_ASIO_RESET_ENABLE)) {
		Error("Can't disable inputs");
		BASS_ASIO_Free();
		BASS_Free();
		return 1;
	}
	
	if (!BASS_ASIO_ChannelReset(FALSE, -1, BASS_ASIO_RESET_ENABLE)) {
		Error("Can't disable outputs");
		BASS_ASIO_Free();
		BASS_Free();
		return 1;
	}
	
	if (!BASS_ASIO_ChannelEnable(TRUE, 1, ASIOProc, NULL)) {
		Error("Can't enable input 1");
		BASS_ASIO_Free();
		BASS_Free();
		return 1;
	}
	
	if (!BASS_ASIO_ChannelEnable(FALSE, 0, ASIOProc, 0)) {
		Error("Can't enable output 0");
		BASS_ASIO_Free();
		BASS_Free();
		return 1;
	}
	
	if (!BASS_ASIO_ChannelSetFormat(TRUE, 1, BASS_ASIO_FORMAT_FLOAT)) {
		Error("Can't set input 1 format to float");
	}
	
	if (!BASS_ASIO_ChannelSetFormat(FALSE, 0, BASS_ASIO_FORMAT_FLOAT)) {
		Error("Can't set output 0 format to float");
	}
	
	chan = BASS_StreamCreate(48000, 1, BASS_SAMPLE_FLOAT|BASS_STREAM_DECODE, STREAMPROC_PUSH, NULL);
	if (!chan) {
		Error("Couldn't create audio stream");
		BASS_ASIO_Free();
		BASS_Free();
		
		return 1;
	}
	
	if (!BASS_ASIO_Start(0,0)) {
		Error("Can't start device");
		BASS_ASIO_Free();
		BASS_Free();
		return 1;
	}
	
	LARGE_INTEGER freq,start,end;
	QueryPerformanceFrequency(&freq); 
	QueryPerformanceCounter(&start);
	
	double elapsed = 0;
	
	printf("Listening...\n");
	while(elapsed < 60) {
		QueryPerformanceCounter(&end);
		elapsed = (end.QuadPart-start.QuadPart)/freq.QuadPart;
		printf("t: %.2f\r",elapsed);
	}
	
	if (!BASS_ASIO_Stop()) {
		Error("Couldn't stop device");
	}

	BASS_ASIO_Free();
	BASS_Free();
	
	return 0;
}
