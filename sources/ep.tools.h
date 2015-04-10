// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

#include "stdlib.h"

//**************************************************************************************

// some usefull fonction, painfull to rewrite over the time (based on taptools_lib.c) :

// Constrain Longs
long EP_clip(long a, long b, long c)
{
	return a < b? b: (a > c? c : a);
}

// Constrain Floats
float EP_fclip(float a, float b, float c)
{
	return a < b? b: (a > c? c : a);
}

// Constrain Doubles
double EP_dclip(double a, double b, double c)
{
	return a < b? b: (a > c? c : a);
}

//************************************************

// Rounding function for longs
long EP_round(long value)
{
	if(value>0) return(value + 0.5);
	else return(value - 0.5);	
}

// Rounding function for floats
float EP_fround(float value)
{
	if(value>0) return(value + 0.5);
	else return(value - 0.5);	
}

// Rounding function for doubles
double EP_dround(double value)
{
	if(value>0) return(value + 0.5);
	else return(value - 0.5);	
}

//************************************************

// Scale function for longs
long EP_scale(long value, long inlow, long inhigh, long outlow, long outhigh)
{
	double inscale, outdiff;
	
 	inscale = 1 / (inhigh - inlow);
 	outdiff = outhigh - outlow;
 	
	value = (value - inlow) * inscale;
	value = (value * outdiff) + outlow;
	return(value);											
}

// Scale function for floats
float EP_fscale(float value, float inlow, float inhigh, float outlow, float outhigh)
{
	float inscale, outdiff;
	
 	inscale = 1 / (inhigh - inlow);
 	outdiff = outhigh - outlow;
 	
	value = (value - inlow) * inscale;
	value = (value * outdiff) + outlow;
	return(value);											
}

// Scale function for doubles
double EP_dscale(double value, double inlow, double inhigh, double outlow, double outhigh)
{
	double inscale, outdiff;
	
 	inscale = 1 / (inhigh - inlow);
 	outdiff = outhigh - outlow;
 	
	value = (value - inlow) * inscale;
	value = (value * outdiff) + outlow;
	return(value);											
}

//**************************************************************************************

// decide function
short EP_decide()
{	
	if (rand() >= RAND_MAX/2)
    {
		return 0;
	}
	else {
		return 1;
	}
}

short EP_pdecide(double percent)
{	
	percent = EP_dclip(percent, 0, 100);
	if (rand() >= RAND_MAX/(100/percent) || percent == 0)
    {
		return 0;
	}
	else
    {
		return 1;
	}
}

//**************************************************************************************
