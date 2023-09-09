#ifndef UAUDIO_H
#define UAUDIO_H

#include <bandpass.h>

#define MAX_GAIN 5
#define EQ_BANDS 10

enum CLIP_TYPES { OFF,HARD,SOFT,NCLIPTYPES };

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Structs
typedef struct audio_struct {
	int bass_status, bassfx_status, asio_status;
	int active;
} audio_struct;
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Functions
int			 	init_audio();
void			start_playback();
void 			set_dsp(int,float,float,float);
void 			remove_dsp();
void 			stop_devices();
void 			toggle_tuner();
void 			tune();
void 			destroy_audio();
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Globals
extern audio_struct* audio_;
extern float master_vol,gain_pos,gain_neg,clip_pos,clip_neg,feedback;
extern float eq[];
extern struct bandpass_struct bp[];
extern int eq_active,type_pos,type_neg,tune_mode;
extern const char* eq_strings[];
extern const char* clip_strings[];
////////////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //UAUDIO_H