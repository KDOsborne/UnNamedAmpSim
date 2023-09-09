#include <usystem.h>
#include <uvideo.h>
#include <utext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TIMELEN 8

struct message_ {
	char* message;
	char timestamp[9];
	struct message_* tail;
	int newlines;
};

static struct message_* sys_mess = NULL;
static float scroll_y = 0.f;
static float height = 0.f;

static void clear_messages();

void post_message(const char* m) {
	
	if(m == NULL) {
		clear_messages();
	} else {
		time_t time_now = time(NULL);
		struct tm date = *localtime(&time_now);
		struct message_* temp = malloc(sizeof(struct message_));
		
		temp->message = malloc(strlen(m)+1);
		memcpy(temp->message,m,strlen(m)+1);
		sprintf(temp->timestamp,"%02d:%02d:%02d",date.tm_hour,date.tm_min,date.tm_sec);
		temp->newlines = 0;
		temp->tail = sys_mess;
	
		for(int i = 0; i < strlen(m); i++) {
			if(m[i] == '\n')
				temp->newlines++;
		}
		
		height += (TXT_SIZE*1.5)*(temp->newlines+1);
		
		sys_mess = temp;
	}
	
	video_->shouldReload_ = 1;
}

void render_messages() {
	struct message_* iter = sys_mess;
	float x = -0.99, y = -0.9+scroll_y, fs = 2.f;
	
	while(iter != NULL) {
		float yoffset = (TXT_SIZE*1.5)*iter->newlines*fs;
		render_simpletext(iter->timestamp,x,y+yoffset,fs,TXT_BOTALIGNED);
		render_simpletext(iter->message,x+(TXT_SIZE*8*fs),y+yoffset,fs,TXT_BOTALIGNED);
		
		y += ((TXT_SIZE*2.f)*fs+yoffset);
		iter = iter->tail;
	}
}

void scroll_messages(float n) {
	scroll_y += n;
	if(scroll_y < 0.f)
		scroll_y = 0.f;
	else if(-1.f+scroll_y > height) {
		scroll_y = height;
	}
}

static void clear_messages() {
	struct message_* temp;
	while(sys_mess != NULL) {
		temp = sys_mess;
		sys_mess = sys_mess->tail;
		
		free(temp->message);
		free(temp);
	}
	height = 0;
}