#ifndef UDIAL_H
#define UDIAL_H

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Structs
struct dial {
	float 			x,y,w,h,lim;
	void* 			value;
	const char**	labels;
	char* 			name;
	int 			type;
	struct dial* 	tail;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Functions
void 			add_dial(float,float,float,float,float,void*,const char**,const char*,int);
void			draw_dials();
struct dial*	click_dials(float,float);
void			destroy_dials();
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Globals
extern struct dial* dials_;
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //UDIAL_H