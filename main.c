#include <windows.h>
#include <uvideo.h>
#include <uaudio.h>
#include <udial.h>
#include <utext.h>
#include <ushape.h>
#include <usystem.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static struct dial* cdial;
static struct dial* tempdial;
static char 		cbuf[64];
static char 		ccopy[64];
static char 		buffer[128];
static int 			mclick;
static const char* command_list = "COMMAND LIST:\n"
	"\4START\1 - START MONITORING\n"
	"\4STOP\1 - STOP MONITORING\n"
	"\4TUNE\1 - START/STOP LIVE TUNER\n"
	"\4EXIT/QUIT\1 - CLOSE PROGRAM"
	"\0";
	
static void print_commands() {
	post_message(command_list);
}

static int process_commands() {
	if(strlen(cbuf) > 1) {
		memset(ccopy,0,sizeof(ccopy));
		memcpy(ccopy,cbuf+1,strlen(cbuf));
		for(int i = 0; i < strlen(cbuf); i++) {
			if(ccopy[i] == ' ') {
				ccopy[i] = '\0';
				break;
			}
		}
		
		if(strcmp(ccopy,"QUIT") == 0 || strcmp(ccopy,"EXIT") == 0)
			return 1;
		else if(strcmp(ccopy,"START") == 0)
			start_playback();
		else if(strcmp(ccopy,"STOP") == 0)
			stop_devices();
		else if(strcmp(ccopy,"TUNE") == 0)
			toggle_tuner();
		else if(strcmp(ccopy,"HELP") == 0)
			print_commands();
		else {
			sprintf(buffer,"UNKNOWN COMMAND: \"%s\"",ccopy);
			post_message(buffer);
		}
		
		cbuf[1] = '\0';
	}
	
	return 0;
}

static int process_messages() {
	MSG msg;
	POINT mousepos;
	float mx,my,v;
	int result_ = 0;
	
	while(PeekMessage(&msg, video_->hWnd, 0, 0, PM_NOREMOVE)) {
		if(GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg); 
			
			switch(msg.message) {
				case WM_CHAR:
					if((isalnum(msg.wParam) || ((msg.wParam == ' ' || msg.wParam == '_' || msg.wParam == '.') && cbuf[1] != '\0')) && strlen(cbuf)+1 < sizeof(cbuf))
					{
						sprintf(cbuf,"%s%c",cbuf,toupper(msg.wParam));
						video_->shouldReload_ = 1;
					}
				break;
				case WM_KEYDOWN:
				video_->shouldReload_ = 1;
				switch (msg.wParam) {
					case VK_ESCAPE:			
						cbuf[1] = '\0';
						
					break;
					case VK_BACK:
						if(strlen(cbuf) > 1)
						{
							cbuf[strlen(cbuf)-1] = '\0';
						}
					break;
					case VK_RETURN:
						result_ = process_commands();
					break;
					}
					case WM_MOUSEMOVE:
						GetCursorPos(&mousepos);
						ScreenToClient(video_->hWnd,&mousepos);
						mx = (mousepos.x/(float)video_->w*2.f-1.f);
						my = -(mousepos.y/(float)video_->h*2.f-1.f);
						
						if(!cdial) {
							if((tempdial = click_dials(mx,my)) != NULL) {
								set_cursor(2);
							} else {
								set_cursor(1);
							}
						} else if (mclick) {
							v = (my-(cdial->y-cdial->h))/(cdial->h*2)*cdial->lim;
							if(v < 0)
								v = 0;
							else if(v > cdial->lim)
								v = cdial->lim;
							
							switch(cdial->type) {
								case 0:
									*((float*)cdial->value) = v;
								break;
								case 1:
									*((int*)cdial->value) = (int)round(v);
								break;
							}
							
							video_->shouldReload_ = 1;
						}
					break;
					case WM_LBUTTONDOWN:
						if(!mclick) {
							if(tempdial) {
								cdial = tempdial;
							}
							mclick = 1;
						} 
						
					break;
					case WM_LBUTTONUP:
						mclick = 0;
						cdial = NULL;
					break;
				break;
			}
			
			DispatchMessage(&msg); 
		}
		else
			result_ = -1;
	}
	return result_;
}

int main(int argc, char* argv[])
{	
	memset(cbuf,0,sizeof(cbuf));
	
	cbuf[0] = '>';
	
	video_ = NULL;
	text_ = NULL;
	shapes_ = NULL;
	audio_ = NULL;
	
	if(!init_video()) {
		return 1;
	}
	
	set_cursor(1);
	
	
	if(!init_text()) {
		destroy_video();
		return 1;
	}
	
	
	if(!init_shapes()) {
		destroy_text();
		destroy_video();
		return 1;
	}
	
	master_vol = 1.f;
	gain_pos = 1.f;
	gain_neg = 1.f;
	clip_pos = 1.f;
	clip_neg = 1.f;
	feedback = 0.f;
	type_pos = OFF;
	type_neg = OFF;
	eq_active = 1;
	tune_mode = 0;
	
	for(int i = 0; i < EQ_BANDS; i++) {
		eq[i] = 1.f;
		init_bandpass(48000,15.625*pow(2,i+1),15.625*pow(2,i),&bp[i]);
	}

	cdial = NULL;
	tempdial = NULL;
	
	float dx = -0.25, dxt = 0.125, dy = 0.5, dw = 0.025, dh = 0.25;
	
	add_dial(dx,dy,dw,dh,1.f,&master_vol,NULL,"VOL",0);
	dx += dxt;
	add_dial(dx,dy,dw,dh,MAX_GAIN,&gain_pos,NULL,"GN+",0);
	dx += dxt;
	add_dial(dx,dy,dw,dh,MAX_GAIN,&gain_neg,NULL,"GN-",0);
	dx += dxt;
	add_dial(dx,dy,dw,dh,1.f,&clip_pos,NULL,"CLP+",0);
	dx += dxt;
	add_dial(dx,dy,dw,dh,1.f,&clip_neg,NULL,"CLP-",0);
	dx += dxt;
	add_dial(dx,dy,dw,dh,NCLIPTYPES-1,&type_pos,clip_strings,"ALG+",1);
	dx += dxt;
	add_dial(dx,dy,dw,dh,NCLIPTYPES-1,&type_neg,clip_strings,"ALG-",1);
	dx += dxt;
	add_dial(dx,dy,dw,dh,0.25,&feedback,NULL,"FEED",0);
	
	dx = -0.25;
	for(int i = 0; i < EQ_BANDS; i++) {
		add_dial(dx+i*dxt,-0.5,dw,dh,1.f,&eq[i],NULL,eq_strings[i],0);
	}
	
	post_message("UNAS SYSTEM STARTED.");
	post_message("TYPE \4HELP \1TO LIST COMMANDS.");
	
	if(!init_audio()) {

	}

	LARGE_INTEGER freq,start,end;
	QueryPerformanceFrequency(&freq); 
	QueryPerformanceCounter(&start);
	
	double elapsed = 0, lastdraw = 0;
	int return_code = 0;
	while(!return_code) {
		QueryPerformanceCounter(&end);
		elapsed = (end.QuadPart-start.QuadPart)*1000/freq.QuadPart;
		
		if(video_->shouldReload_) {
			glClear(GL_COLOR_BUFFER_BIT);
			
			render_messages();
			render_simpletext(cbuf,-0.99,-0.9625,2.5,0);
			render_simpletext("UNNAMED AMP SIM V0.1",1.f,0.99,3.f,TXT_TOPALIGNED|TXT_RGHTALIGNED);
			draw_line(0.f,-0.925,1.f,0);
			draw_dials();
			
			if(tune_mode) {
				tune();
			}
			
			SwapBuffers(video_->hDC);
			lastdraw = elapsed;
			video_->shouldReload_ = 0;
		}
		
		return_code = process_messages();
		
		if(!audio_->active)
			Sleep(1);
	}
	
	post_message(NULL);
	
	destroy_audio();
	destroy_dials();
	destroy_shapes();
	destroy_text();
	destroy_video();
	
	return 0;
}
