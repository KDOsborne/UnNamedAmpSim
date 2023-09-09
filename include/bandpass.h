#ifndef BANDPASS_H
#define BANDPASS_H

struct bandpass_struct {
	double A,d1,d2,d3,d4,w0,w1,w2,w3,w4;
};

void init_bandpass(double,double,double,struct bandpass_struct*);

#endif //BANDPASS_H