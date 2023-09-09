#include <udial.h>
#include <uvideo.h>
#include <ushape.h>
#include <utext.h>

#include <stdio.h>

struct dial* dials_;

void add_dial(float x,float y,float w,float h,float l,void* v,const char** labels,const char* name,int t) {
	struct dial* temp = malloc(sizeof(struct dial));
	
	temp->x = x;
	temp->y = y;
	temp->w = w;
	temp->h = h;
	temp->lim = l;
	temp->value = v;
	temp->labels = labels;
	temp->name = malloc(strlen(name)+1);
	memcpy(temp->name,name,strlen(name)+1);
	temp->type = t;
	temp->tail = dials_;
	
	dials_ = temp;
}
	
void draw_dials() {
	struct dial* iter = dials_;
	char buf[8];
	
	while(iter != NULL) {
		draw_line(iter->x,iter->y,iter->h,1);
		
		switch(iter->type) {
			case 0:
				draw_rectangle(iter->x,(iter->y-iter->h)+(*((float*)iter->value))/iter->lim*iter->h*2,iter->w,iter->w/2,0);
				if(*((float*)iter->value) == iter->lim)
					sprintf(buf,"MAX");
				else
					sprintf(buf,"%.2f",*((float*)iter->value));
			break;
			case 1:
				draw_rectangle(iter->x,(iter->y-iter->h)+(*((int*)iter->value))/iter->lim*iter->h*2,iter->w,iter->w/2,0);
				sprintf(buf,"%s",iter->labels[*((int*)iter->value)]);
			break;
		}
		
		render_simpletext(iter->name,iter->x,iter->y+iter->h+iter->w,2.5,TXT_BOTALIGNED|TXT_CENTERED);
		render_simpletext(buf,iter->x,iter->y-iter->h-iter->w,2.5,TXT_TOPALIGNED|TXT_CENTERED);
		
		iter = iter->tail;
	}
}

struct dial* click_dials(float x, float y) {
	struct dial* iter = dials_;
	
	while(iter != NULL) {
		float ypos = 0.f;
		switch(iter->type) {
			case 0:
				ypos = (iter->y-iter->h)+(*((float*)iter->value))/iter->lim*iter->h*2;
			break;
			case 1:
				ypos = (iter->y-iter->h)+(*((int*)iter->value))/iter->lim*iter->h*2;
			break;
		}
		
		if(x >= iter->x-iter->w && x <= iter->x+iter->w && y >= ypos-iter->w/2 && y <= ypos+iter->w/2) {
			return iter;
		}
		iter = iter->tail;
	}
	
	return NULL;
}

void destroy_dials() {
	struct dial* temp;
	while(dials_ != NULL) {
		temp = dials_;
		dials_ = dials_->tail;
		free(temp->name);
		free(temp);
	}
}