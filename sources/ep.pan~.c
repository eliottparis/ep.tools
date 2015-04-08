
// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

/**

 simple panning a signal into two signals.
 
 */

#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"

#include "../EP_EXTERNS.h"
#define OBJECT_NAME "ep.pan~"

#define FADETIME_INIT 20

void *pan_class;

typedef struct _pan
{
    t_pxobject x_obj;
	
	t_float x_LFromValue;
	t_float x_RFromValue;
	t_float x_LToValue;
	t_float x_RToValue;
	t_float x_LValue;
	t_float x_RValue;
	t_float x_LSmoothFactor;
	t_float x_RSmoothFactor;
	t_float x_LSampsToFade;
	t_float x_RSampsToFade;
	t_float x_fadeTime;
	t_int x_fadeSamps;
	t_float x_sr;
	t_int x_Lcon; // Left connected ?
	
} t_pan;

void *pan_new(double val);
t_int *offset_perform(t_int *w);
t_int *pan_perform(t_int *w);

void pan_SetL(t_pan *x, double f);
void pan_SetR(t_pan *x, double f);
void samps_to_fade(t_pan *x);
void setFadeTime(t_pan *x, double f);

void pan_float(t_pan *x, double f);
void pan_int(t_pan *x, long n);
void pan_list(t_pan *x, t_symbol *s, long ac, t_atom *av);
void pan_dsp(t_pan *x, t_signal **sp, short *count);
void pan_assist(t_pan *x, void *b, long m, long a, char *s);

int C74_EXPORT main(void)
{
    t_class *c;

	c = class_new("ep.pan~", (method)pan_new, (method)dsp_free, (short)sizeof(t_pan), 0L, A_DEFFLOAT, 0);
    
    class_addmethod(c, (method)pan_dsp, "dsp", A_CANT, 0);				// respond to the dsp message 
																			// (sent to MSP objects when audio is turned on/off)
    class_addmethod(c, (method)pan_float, "float", A_FLOAT, 0);
    class_addmethod(c, (method)pan_int, "int", A_LONG, 0);
	class_addmethod(c, (method)pan_list, "list", A_GIMME, 0);
	class_addmethod(c, (method)pan_assist,"assist",A_CANT,0);
    class_dspinit(c);														// must call this function for MSP object classes

	class_register(CLASS_BOX, c);
	pan_class = c;
	
	post("%s %s", OBJECT_NAME, EP_EXTERNS_MSG);
	
	return 0;
}

void pan_assist(t_pan *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) {
		switch (a) {	
		case 0:
			sprintf(s,"(Signal) In");
			break;
		case 1:
			sprintf(s,"(Float/int) balance");
			break;
		}
	} 
	else {
		switch (a) {	
			case 0:
				sprintf(s,"(Signal) Left");
				break;
			case 1:
				sprintf(s,"(Signal) Right");
				break;
		}
	}
}

void *pan_new(double val)
{
    t_pan *x = object_alloc(pan_class);
    dsp_setup((t_pxobject *)x,1);					// set up DSP for the instance and create 2 signal inlets
    outlet_new((t_pxobject *)x, "signal");			// signal outlets are created like this
	outlet_new((t_pxobject *)x, "signal");
	
	x->x_sr = sys_getsr();							// get sampling rate and store it
	if(!x->x_sr){
		x->x_sr = 44100.0;
		error("zero sampling rate - set to 44100");
	}
	
	x->x_fadeTime = FADETIME_INIT / 1000.;
	x->x_fadeSamps = x->x_fadeTime * x->x_sr;
	
	//post("sampling rate = %f, fadeSamps = %ld", x->x_sr, x->x_fadeSamps);
	
	x->x_LValue = x->x_RValue = 0.5;
	
    return (x);
}

void setFadeTime(t_pan *x, double f)
{
	if (f <= 5) {
		x->x_fadeTime = 5. / 1000.;
	}
	else {
		x->x_fadeTime = f / 1000.;
	}
}

void samps_to_fade(t_pan *x)
{
	x->x_sr = sys_getsr();							// get sampling rate and store it
	if(!x->x_sr){
		x->x_sr = 44100.0;
		error("zero sampling rate - set to 44100");
	}
	
	x->x_fadeSamps = x->x_fadeTime * x->x_sr;

}

void pan_SetL(t_pan *x, double f)
{
	x->x_LFromValue = x->x_LValue;
	x->x_LToValue = f;
	
	samps_to_fade(x);
	x->x_LSmoothFactor = ((x->x_LToValue - x->x_LFromValue) / x->x_fadeSamps );
	x->x_LSampsToFade = x->x_fadeSamps;
}

void pan_SetR(t_pan *x, double f)
{
	x->x_RFromValue = x->x_RValue;
	x->x_RToValue = f;
	
	samps_to_fade(x);
	x->x_RSmoothFactor = ((x->x_RToValue - x->x_RFromValue) / x->x_fadeSamps );
	x->x_RSampsToFade = x->x_fadeSamps;
}

void pan_list(t_pan *x, t_symbol *s, long ac, t_atom *av)
{
	if (ac == 2) {
		
		if ((x->x_LSampsToFade <= 0) && (x->x_RSampsToFade <= 0)) {
			if ((av+1)->a_type == A_LONG) {
				setFadeTime(x, atom_getlong(av+1));
			}
			if ((av+1)->a_type == A_FLOAT) {
				setFadeTime(x, atom_getfloat(av+1));
			}
		}
		
		if ((av)->a_type == A_LONG) {
			pan_float(x, atom_getlong(av));
		}
		if ((av)->a_type == A_FLOAT) {
			pan_float(x, atom_getfloat(av));
		}
	}
}

void pan_float(t_pan *x, double f)						// the float and int routines cover both inlets. 
{														// It doesn't matter which one is involved
	
	double balance = EP_dclip(f, 0, 1);
	
	pan_SetR(x, balance);
	pan_SetL(x, 1 - balance);
	
	//post("in connected? : %ld", x->x_Lcon);
}

void pan_int(t_pan *x, long n)
{
	pan_float(x, n);
}

t_int *offset_perform(t_int *w)						// our perform method if both signal inlets are connected
{
	
	t_float *outL,*outR;
	int n;
	//int i = 0;
	
	if (*(long *)(w[1]))
	    goto out;
	
	t_pan *x = (t_pan *)(w[2]);
	outL = (t_float *)(w[3]);
	outR = (t_float *)(w[4]);
	n = (int)(w[5]);

	while (n--) {
		*outR++ = 0.0;
		*outL++ = 0.0;
	}
	
out:

	return (w+6);
}

t_int *pan_perform(t_int *w)						// our perform method if both signal inlets are connected
{
	t_float *inL,*outL,*outR;
	int n;
	int i = 0;

	if (*(long *)(w[1]))
	    goto out;

	t_pan *x = (t_pan *)(w[2]);
	inL = (t_float *)(w[3]);
	outL = (t_float *)(w[4]);
	outR = (t_float *)(w[5]);
	n = (int)(w[6]);
	
	/*
	if (x->x_Lcon == 0){
	    goto out;
	}
	*/
	
	for (i=0; i<n; i++, outL++, outR++) {
		
		if (x->x_RSampsToFade > 0) {
			
			x->x_RValue = ( x->x_RFromValue + (x->x_RSmoothFactor * (x->x_fadeSamps - x->x_RSampsToFade) ) );
			--x->x_RSampsToFade;
		}
		
		if (x->x_LSampsToFade > 0) {
			
			x->x_LValue = ( x->x_LFromValue + (x->x_LSmoothFactor * (x->x_fadeSamps - x->x_LSampsToFade) ) );
			--x->x_LSampsToFade;
		}
		
		*outR = (*inL * x->x_RValue);
		*outL = (*inL * x->x_LValue);
		
		inL++;
	}
	
	if ((x->x_LSampsToFade <= 0) && (x->x_RSampsToFade <= 0)) {
		setFadeTime(x, FADETIME_INIT);
	}
	
out:
	
	return (w+7);
}		

void pan_dsp(t_pan *x, t_signal **sp, short *count)	// method called when dsp is turned on
{
	x->x_Lcon = count[0]; // Left connected ?
	
	if (count[0]) {
		dsp_add(pan_perform, 6, &x->x_obj.z_disabled, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
	}
	else {
		dsp_add(offset_perform, 5, &x->x_obj.z_disabled, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
	}
}

