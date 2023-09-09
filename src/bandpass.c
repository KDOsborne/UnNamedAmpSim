/*
 *                            COPYRIGHT
 *
 *  Copyright (C) 2014 Exstrom Laboratories LLC
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  A copy of the GNU General Public License is available on the internet at:
 *  http://www.gnu.org/copyleft/gpl.html
 *
 *  or you can write to:
 *
 *  The Free Software Foundation, Inc.
 *  675 Mass Ave
 *  Cambridge, MA 02139, USA
 *
 *  Exstrom Laboratories LLC contact:
 *  stefan(AT)exstrom.com
 *
 *  Exstrom Laboratories LLC
 *  Longmont, CO 80503, USA
 *
 */

#include <bandpass.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

void init_bandpass(double s, double f1, double f2, struct bandpass_struct* bp)
{
	double a = cos(M_PI*(f1+f2)/s)/cos(M_PI*(f1-f2)/s);
	double a2 = a*a;
	double b = tan(M_PI*(f1-f2)/s);
	double b2 = b*b;
	double r;
	
	r = sin(M_PI*(1.0)/(4.0));
	s = b2 + 2.0*b*r + 1.0;
	bp->A = b2/s;
	bp->d1 = 4.0*a*(1.0+b*r)/s;
	bp->d2 = 2.0*(b2-2.0*a2-1.0)/s;
	bp->d3 = 4.0*a*(1.0-b*r)/s;
	bp->d4 = -(b2-2.0*b*r+1.0)/s;
	
	bp->w0 = 0;
	bp->w1 = 0;
	bp->w2 = 0;
	bp->w3 = 0;
	bp->w4 = 0;
}