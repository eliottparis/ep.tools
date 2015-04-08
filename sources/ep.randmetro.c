
// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

#include "ext.h"
#include "ext_obex.h"
#include "stdlib.h"

#define DEFAULT_TEMPO	1000
#define MIN_TEMPO		10

typedef struct _randmetro {	// data structure for this object
	t_object	m_ob;		// must always be the first field; used by Max
	void *m_clock;			// pointer to clock object
	long m_interval;		// tempo in milliseconds
	long m_Min;				// Min Interval
	long m_Max;				// Max Interval
	short m_mode;			// 0 = normal / 1 = Random Interval
	short m_activ;			// is active ?
	void *m_bang_outlet;	// pointers to bang outlet
	void *m_time_outlet;	// pointers to time outlet
} t_randmetro;


void *randmetro_new(long Min, long Max);
void randmetro_int(t_randmetro *x, long togval);
void randmetro_in1(t_randmetro *x, long Min);
void randmetro_in2(t_randmetro *x, long Max);
void randmetro_bang(t_randmetro *x);
void randmetro_assist(t_randmetro *x, void *b, long m, long a, char *s);
void randmetro_free(t_randmetro *x);
void randmetro_stop(t_randmetro *x);
void randmetro_start(t_randmetro *x);
void randmetro_newRandInterval(t_randmetro *x);
void clock_function(t_randmetro *x);
void *randmetro_class;				// Required. Global pointing to this class


int C74_EXPORT main(void) {
	
	t_class *c;
	
	c = class_new("ep.randmetro", (method)randmetro_new, (method)randmetro_free, (long)sizeof(t_randmetro), 
				  0L /* leave NULL!! */, A_DEFLONG, A_DEFLONG, 0);
	
	class_addmethod(c, (method)randmetro_bang,		"bang");
	class_addmethod(c, (method)randmetro_int,		"int",		A_LONG,		0);
	class_addmethod(c, (method)randmetro_in1,		"in1",		A_LONG,		0);
	class_addmethod(c, (method)randmetro_in2,		"in2",		A_LONG,		0);
	//class_addmethod(c, (method)randmetro_float,	"float",	A_FLOAT,	0);
	class_addmethod(c, (method)randmetro_stop,	"stop", 0);
	class_addmethod(c, (method)randmetro_start,	"start", 0);
	
	class_addmethod(c, (method)randmetro_assist,		"assist",	A_CANT,		0);  
	
	class_register(CLASS_BOX, c); /* CLASS_NOBOX */
	randmetro_class = c;
	
	post("ep.randmetro object "__DATE__" by Eliott Paris");
	
	return 0;
}

/***********************************************************************************/

void *randmetro_new(long Min, long Max)
{
	t_randmetro *x;
	x = (t_randmetro *)object_alloc(randmetro_class);	// create the new instance and return a pointer to it
	
	if (Min == 0 && Max == 0) {					// si 0 arg ou si "0 0" args
		x->m_Min = x->m_Max = DEFAULT_TEMPO;
		x->m_interval = DEFAULT_TEMPO;
	}
	
	else if (Min != Max) {
			if (Min > MIN_TEMPO){
				x->m_Min = Min;
			}
			else {
				x->m_Min = MIN_TEMPO;
			}
			if (Max > MIN_TEMPO) {
				x->m_Max = Max;
			}
			else if (Max != 0){
				x->m_Max = MIN_TEMPO;
			}
			else {
				x->m_Max = x->m_Min;
			}
		x->m_interval = x->m_Min;
		}
	
	else {
		x->m_Min = x->m_Max = Min;
		x->m_interval = x->m_Min;
	}

	
	x->m_clock = clock_new(x, (method)clock_function); // create the metronome clock
	intin(x, 2);
	intin(x, 1);					// create the right inlet
	x->m_time_outlet = intout(x);	// create right outlet for time
	x->m_bang_outlet = bangout(x);	// create left outlet for ticks
	
	return(x);
}

/***********************************************************************************/

void randmetro_int(t_randmetro *x, long togval)
{
	if (togval == 0) {
		x->m_activ = 0;
		randmetro_stop(x);
	}
	else {
		x->m_activ = 1;
		randmetro_start(x);
	}
}

void randmetro_in1(t_randmetro *x, long Min) 
{
	if (Min < MIN_TEMPO) {
		x->m_Min = MIN_TEMPO;
	}
	else {
		x->m_Min = Min;
	}
	
	//x->m_interval = Min;	// store the new metronome interval
	//post("metronome tempo changed to %ld", Min);
}


void randmetro_in2(t_randmetro *x, long Max) 
{
	if (Max < MIN_TEMPO) {
		x->m_Max = MIN_TEMPO;
	}
	else {
		x->m_Max = Max;
	}

	//x->m_interval = Max;	// store the new randmetronome interval
	//post("metronome tempo changed to %ld", Max);
}

/***********************************************************************************/ 


void randmetro_bang(t_randmetro *x) 
{
	randmetro_start(x);
}


/***********************************************************************************/

void randmetro_start(t_randmetro *x)
{
	long time;
	time = gettime();	// get current time 
	clock_delay(x->m_clock, 0L); // set clock to go off now 
	//post("clock started at %ld", time);
}

void randmetro_stop(t_randmetro *x) 
{
	long time;
	time = gettime();	// get current time
	clock_unset(x->m_clock); //remove the clock routine from the scheduler
	//outlet_int(x->m_time_outlet, time); 
	//post("metronome stopped at %ld", time);
}

/***********************************************************************************/

void randmetro_newRandInterval(t_randmetro *x)
{
	long randvalue;
	if (x->m_Max > x->m_Min) {
		randvalue = rand()%((x->m_Max +1) - x->m_Min) + x->m_Min;
	}
	else if (x->m_Max < x->m_Min) {
		randvalue = rand()%((x->m_Min +1) - x->m_Max) + x->m_Max;
	}
	x->m_interval = randvalue;
	//post("randvalue = %ld", randvalue);
}

/***********************************************************************************/

void clock_function(t_randmetro *x)
{
	long time;
	time = gettime();						// get current time
	
	if (x->m_Min != x->m_Max) {
		randmetro_newRandInterval(x);
	}
	
	clock_delay(x->m_clock, x->m_interval); // schedule another metronome click
	outlet_bang(x->m_bang_outlet);			// send out a bang
	outlet_int(x->m_time_outlet, x->m_interval);		// send next Interval time to right outlet
	
	//post("clock_function %ld", time);
}

/***********************************************************************************/

void randmetro_free(t_randmetro *x)
{
	clock_unset(x->m_clock); //remove the clock routine from the scheduler
	clock_free(x->m_clock);	// free the clock memory
}

void randmetro_assist(t_randmetro *x, void *b, long m, long a, char *s) 
{
	if (m == ASSIST_INLET) { // inlet
		switch (a) {
			case 0:
				sprintf(s, "Start/Stop Metronome");
				break;
			case 1:
				sprintf(s, "Set Metronome Time Interval");
				break;
			case 2:
				sprintf(s, "Set Max Metronome Time Interval");
				break;
			default:
				break;
		}
	} 
	else {					// outlet
		switch (a) {
			case 1:
				sprintf(s, "Interval time between Next Ticks"); 
				break;
			case 0:
				sprintf(s, "(bang) Ticks"); 
				break;
			default:
				break;
		}
	}	
}
