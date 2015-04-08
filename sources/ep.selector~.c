
// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

/*
 ep.selector~ works like the standard selector~ object of MSP distribution but integrate a crosseFade interpolation fonction.
 based on eric lyon's clean.selector~ external
*/

#define EP_EXTERNALS_MSG "object by Eliott Paris, version Beta "__DATE__""
#define OBJECT_NAME "ep.selector~"

#include "ext.h"
#include "z_dsp.h"
#include "ext_obex.h"
#define resizebytes t_resizebytes

#define MAX_CHANS (64)
#define CS_LINEAR (0)
#define CS_POWER (1)

typedef struct _ep_selector
{
	t_pxobject x_obj;

	// Variables Here
	short input_chans;
	short active_chan;
	short last_chan;
	int samps_to_fade;
	int fadesamps;
	float fadetime;
	float pi_over_two;
	short fadetype;
	short *connected_list;
	float **bulk ; // array to point to all input audio channels
	float sr;
	float vs;
	int inlet_count;
} t_ep_selector;

void *ep_selector_new(t_symbol *s, int argc, t_atom *argv);

t_int *ep_selector_perform(t_int *w);
void ep_selector_dsp(t_ep_selector *x, t_signal **sp, short *count);
void ep_selector_assist(t_ep_selector *x, void *b, long m, long a, char *s);
void ep_selector_float(t_ep_selector *x, t_float f);
void ep_selector_fadetime(t_ep_selector *x, double f);
void ep_selector_int(t_ep_selector *x, t_int i);
void ep_selector_channel(t_ep_selector *x, double i);
void ep_selector_dsp_free(t_ep_selector *x);

t_class *ep_selector_class;

int C74_EXPORT main(void)
{
	t_class *c;
	
	c = class_new("ep.selector~", (method)ep_selector_new, (method)dsp_free, (short)sizeof(t_ep_selector), 0L, A_GIMME, 0);
	
	class_addmethod(c, (method)ep_selector_dsp,			"dsp",		A_CANT, 0);
	class_addmethod(c, (method)ep_selector_assist,		"assist",	A_CANT, 0);
	class_addmethod(c, (method)ep_selector_fadetime,	"fadetime",	A_FLOAT, 0);
	class_addmethod(c, (method)ep_selector_channel,		"channel",	A_FLOAT, 0);
	class_addmethod(c, (method)ep_selector_float,		"float",	A_FLOAT, 0);
	class_addmethod(c, (method)ep_selector_int,			"int",		A_LONG, 0);
	
	class_dspinit(c);				// new style object version of dsp_initclass();
	class_register(CLASS_BOX, c);	// register class as a box class
	ep_selector_class = c;
    
    post("ep.selector~ object by Eliott Paris");
	
	return 0;
}

void ep_selector_assist (t_ep_selector *x, void *b, long msg, long a, char *s)
{
	if (msg==1) {
		if(a == 0){
			sprintf(s,"(signal/int) Input 0, Channel Number");
		} else {
			sprintf(s,"(signal) Input %ld",a);
		}
		
	} else if (msg==2) {
		sprintf(s,"(signal) Output");
	}
}

void *ep_selector_new(t_symbol *s, int argc, t_atom *argv)
{
	int i;
	t_ep_selector *x = (t_ep_selector *)object_alloc(ep_selector_class);
	
	if (x)
    {
		x->fadetime = 0.05;
		x->inlet_count = 2;
		if(argc >= 1){
			x->inlet_count = (int)atom_getfloatarg(0,argc,argv);
			if(x->inlet_count < 2 || x->inlet_count > MAX_CHANS){
				error("%s: please enter a number of inlets between 2 and %ld",OBJECT_NAME, MAX_CHANS);
				return (void *) NULL;
			}
			
		}  
		if(argc >= 2){
			x->fadetime = atom_getfloatarg(1,argc,argv) / 1000.0;
		}
		
		//		post("argc %d inlet count %d fadetime %f",argc, x->inlet_count, x->fadetime);
		
		dsp_setup((t_pxobject *)x, x->inlet_count); 
		outlet_new((t_pxobject *)x, "signal");
		x->x_obj.z_misc |= Z_NO_INPLACE;
		
		x->sr = sys_getsr();
		if(!x->sr){
			x->sr = 44100.0;
			error("zero sampling rate - set to 44100");
		}
		x->fadetype = CS_POWER;
		x->pi_over_two = 1.57079632679;
		
		
		
		if(x->fadetime <= 0.0)
			x->fadetime = .05;
		x->fadesamps = x->fadetime * x->sr;
		
		x->connected_list = (short *) t_getbytes(MAX_CHANS * sizeof(short));
		for(i=0;i<16;i++){
			x->connected_list[i] = 0;
		}
		x->active_chan = x->last_chan = 0;
		x->bulk = (t_float **) t_getbytes(16 * sizeof(t_float *));
		x->samps_to_fade = 0;
	
	}
		
	return (x);
}

void ep_selector_dsp_free(t_ep_selector *x)
{
	dsp_free((t_pxobject *)x);
	t_freebytes(x->bulk, 16 * sizeof(t_float *));
}


void ep_selector_fadetime(t_ep_selector *x, double f)
{
	float fades = (float)f / 1000.0;
	
	if( fades < .0001 || fades > 1000.0 ){
		error("fade time must be between 0.1 - 1000000");
		return;
	}
	x->fadetime = fades;
	x->fadesamps = x->sr * x->fadetime;
	x->samps_to_fade = 0;
}

t_int *ep_selector_perform(t_int *w)
{
	
	t_ep_selector *x = (t_ep_selector *) (w[1]);
	int i;
	t_int n;
	t_float *out;
	
	int fadesamps = x->fadesamps;
	short active_chan = x->active_chan;
	short last_chan = x->last_chan;
	int samps_to_fade = x->samps_to_fade;
	float m1, m2;
	float **bulk = x->bulk;
	float pi_over_two = x->pi_over_two;
	short fadetype = x->fadetype;
	float phase;
	int inlet_count = x->inlet_count;
	
	for ( i = 0; i < inlet_count; i++ ) {
		bulk[i] = (t_float *)(w[2 + i]);
	}
	out = (t_float *)(w[inlet_count + 2]);
	n = w[inlet_count + 3]; 
	
	/********************************************/
	if ( active_chan >= 0 ) {
		while( n-- ) {
			if ( samps_to_fade >= 0 ) {
				if( fadetype == CS_POWER ){
					phase = pi_over_two * (1.0 - (samps_to_fade / (float) fadesamps)) ;
					m1 = sin( phase );
					m2 = cos( phase );
					--samps_to_fade;
					*out++ = (*(bulk[active_chan])++ * m1) + (*(bulk[last_chan])++ * m2);
				} 
			}
			else {
				*out++ =  *(bulk[active_chan])++;
			}
		}
	} 
  	else  {
  		while( n-- ) {
			*out++ = 0.0;
		}
  	}
    
	x->samps_to_fade = samps_to_fade;
	return (w + (inlet_count + 4));
}		

void ep_selector_dsp(t_ep_selector *x, t_signal **sp, short *count)
{
	long i;
	t_int **sigvec;
	int pointer_count;
	
	pointer_count = x->inlet_count + 3; // all inlets, 1 outlet, object pointer and vec-samps	
	sigvec  = (t_int **) calloc(pointer_count, sizeof(t_int *));	
	for(i = 0; i < pointer_count; i++){
		sigvec[i] = (t_int *) calloc(sizeof(t_int),1);
	}
	sigvec[0] = (t_int *)x; // first pointer is to the object
	
	sigvec[pointer_count - 1] = (t_int *)sp[0]->s_n; // last pointer is to vector size (N)

	for(i = 1; i < pointer_count - 1; i++){ // now attach the inlet and all outlets
		sigvec[i] = (t_int *)sp[i-1]->s_vec;
	}
		
	if(x->sr != sp[0]->s_sr){
		x->sr = sp[0]->s_sr;
		x->fadesamps = x->fadetime * x->sr;
		x->samps_to_fade = 0;
	}

	dsp_addv(ep_selector_perform, pointer_count, (void **) sigvec); 
	free(sigvec);

	for (i = 0; i < MAX_CHANS; i++) {
		x->connected_list[i] = count[i];
	}			
}


void ep_selector_float(t_ep_selector *x, t_float f) // Look at floats at inlets
{
	ep_selector_int(x,(t_int)f);	
}

void ep_selector_int(t_ep_selector *x, t_int i) // Look at int at inlets
{
	int inlet = ((t_pxobject*)x)->z_in;
	
	if (inlet == 0)
	{		
			ep_selector_channel(x,(double)i);
	}	

}

void ep_selector_channel(t_ep_selector *x, double i) // Look at int at inlets
{
	int chan = i;
	if(chan < 0 || chan > x->inlet_count - 1){
		post("%s: channel %d out of range",OBJECT_NAME, chan);
		return;
	}	
	if(chan != x->active_chan) {
		
		x->last_chan = x->active_chan;
		x->active_chan = chan;
		x->samps_to_fade = x->fadesamps;
		if( x->active_chan < 0)
			x->active_chan = 0;
		if( x->active_chan > MAX_CHANS - 1) {
			x->active_chan = MAX_CHANS - 1;
		}
		if(! x->connected_list[chan]) {
		// do it anyway - it's user-stupidity
		/*
			post("warning: channel %d not connected",chan);
			x->active_chan = 1; */
		}
		// post("last: %d active %d", x->last_chan, x->active_chan);
	}	
}
