
// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

/**

 simple crossfading between two signals.
 
 */

#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"

#include "../EP_EXTERNS.h"
#define OBJECT_NAME "ep.xfade~"

#define FADETIME_INIT 5

void *xfade_class;

typedef struct _xfade
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
	t_int x_Rcon; // Right connected ?
	
} t_xfade;

void *xfade_new(double val);
t_int *offset_perform(t_int *w);
t_int *xfade2_perform(t_int *w);

void xfade_SetL(t_xfade *x, double f);
void xfade_SetR(t_xfade *x, double f);
void samps_to_fade(t_xfade *x);
void setFadeTime(t_xfade *x, double f);

void xfade_float(t_xfade *x, double f);
void xfade_int(t_xfade *x, long n);
void xfade_list(t_xfade *x, t_symbol *s, long ac, t_atom *av);
void xfade_dsp(t_xfade *x, t_signal **sp, short *count);
void xfade_assist(t_xfade *x, void *b, long m, long a, char *s);

int main(void)
{
    t_class *c;

	c = class_new("ep.xfade~", (method)xfade_new, (method)dsp_free, (short)sizeof(t_xfade), 0L, A_DEFFLOAT, 0);
    
    class_addmethod(c, (method)xfade_dsp, "dsp", A_CANT, 0);				// respond to the dsp message 
																			// (sent to MSP objects when audio is turned on/off)
    class_addmethod(c, (method)xfade_float, "float", A_FLOAT, 0);
    class_addmethod(c, (method)xfade_int, "int", A_LONG, 0);
	class_addmethod(c, (method)xfade_list, "list", A_GIMME, 0);
	class_addmethod(c, (method)xfade_assist,"assist",A_CANT,0);
    class_dspinit(c);														// must call this function for MSP object classes

	class_register(CLASS_BOX, c);
	xfade_class = c;
	
	post("%s %s", OBJECT_NAME, EP_EXTERNS_MSG);
	
	return 0;
}

void xfade_assist(t_xfade *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) {
		switch (a) {	
		case 0:
			sprintf(s,"(Signal) Left + Mix Value");
			break;
		case 1:
			sprintf(s,"(Signal) Right");
			break;
		}
	} 
	else {
		switch (a) {	
			case 0:
				sprintf(s,"(Signal) Mix Left + Right");
				break;
		}
	}
}

void *xfade_new(double val)
{
    t_xfade *x = object_alloc(xfade_class);
    dsp_setup((t_pxobject *)x,2);					// set up DSP for the instance and create 2 signal inlets
    outlet_new((t_pxobject *)x, "signal");			// signal outlets are created like this
	
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

void setFadeTime(t_xfade *x, double f)
{
	if (f <= 5) {
		x->x_fadeTime = 5. / 1000.;
	}
	else {
		x->x_fadeTime = f / 1000.;
	}
}

void samps_to_fade(t_xfade *x)
{
	x->x_sr = sys_getsr();							// get sampling rate and store it
	if(!x->x_sr){
		x->x_sr = 44100.0;
		error("zero sampling rate - set to 44100");
	}
	
	x->x_fadeSamps = x->x_fadeTime * x->x_sr;

}

void xfade_SetL(t_xfade *x, double f)
{
	x->x_LFromValue = x->x_LValue;
	x->x_LToValue = f;
	
	samps_to_fade(x);
	x->x_LSmoothFactor = ((x->x_LToValue - x->x_LFromValue) / x->x_fadeSamps );
	x->x_LSampsToFade = x->x_fadeSamps;
}

void xfade_SetR(t_xfade *x, double f)
{
	x->x_RFromValue = x->x_RValue;
	x->x_RToValue = f;
	
	samps_to_fade(x);
	x->x_RSmoothFactor = ((x->x_RToValue - x->x_RFromValue) / x->x_fadeSamps );
	x->x_RSampsToFade = x->x_fadeSamps;
}

void xfade_list(t_xfade *x, t_symbol *s, long ac, t_atom *av)
{
	if (ac == 2) {
		
		if ((x->x_LSampsToFade <= 0) && (x->x_RSampsToFade <= 0)) {
			if ((av+1)->a_type == A_LONG) {
				setFadeTime(x, atom_getlong(av+1));			}
			if ((av+1)->a_type == A_FLOAT) {
				setFadeTime(x, atom_getfloat(av+1));
			}
		}
		
		if ((av)->a_type == A_LONG) {
			xfade_float(x, atom_getlong(av));
		}
		if ((av)->a_type == A_FLOAT) {
			xfade_float(x, atom_getfloat(av));
		}
	}
}

void xfade_float(t_xfade *x, double f)						// the float and int routines cover both inlets. 
{														// It doesn't matter which one is involved
	double balance = f;
	
	if (balance <= 0) {
		balance = 0;
	}
	else if (balance >= 1) {
		balance = 1;
	}
	
	xfade_SetR(x, balance);
	xfade_SetL(x, 1 - balance);
}

void xfade_int(t_xfade *x, long n)
{
	xfade_float(x, n);
}

t_int *offset_perform(t_int *w)							// our perform method if one signal inlet is connected
{														// a optimiser => signal mono dans outL qd gauche connecte, 0 signal si seulement droit..!
    t_float *inL = (t_float *)(w[1]);
	t_float *inR = (t_float *)(w[2]);
    t_float *outL = (t_float *)(w[3]);
	t_xfade *x = (t_xfade *)(w[4]);
	int n = (int)(w[5]);
	long i;
	
	if (x->x_obj.z_disabled)
		goto out;
	
	else if (x->x_Lcon) {								// si seul le canal gauche est connecte => Mono L = R = inL
		for (i=0; i<n; i++, outL++) {
			if (x->x_LSampsToFade > 0) {
				
				x->x_LValue = ( x->x_LFromValue + (x->x_LSmoothFactor * (x->x_fadeSamps - x->x_LSampsToFade) ) );
				--x->x_LSampsToFade;
			}
			*outL = (*inL * x->x_LValue);
			inL++;
		}
	}
	else {												// si seul le canal droit est connecte => Mono L = R = inR
		for (i=0; i<n; i++, outL++) {
			if (x->x_RSampsToFade > 0) {
				
				x->x_RValue = ( x->x_RFromValue + (x->x_RSmoothFactor * (x->x_fadeSamps - x->x_RSampsToFade) ) );
				--x->x_RSampsToFade;
			}
			*outL = (*inR * x->x_RValue);
			inR++;
		}
	}
	
	if ((x->x_LSampsToFade <= 0) && (x->x_RSampsToFade <= 0)) {
		setFadeTime(x, FADETIME_INIT);
	}
	
out:
    return (w+6);
}

t_int *xfade2_perform(t_int *w)						// our perform method if both signal inlets are connected
{
	t_float *inL,*inR,*outL;
	int n;
	int i = 0;

	if (*(long *)(w[1]))
	    goto out;

	t_xfade *x = (t_xfade *)(w[2]);
	inL = (t_float *)(w[3]);
	inR = (t_float *)(w[4]);
	outL = (t_float *)(w[5]);
	n = (int)(w[6]);
	
	for (i=0; i<n; i++, outL++) {
		
		if (x->x_RSampsToFade > 0) {
			
			x->x_RValue = ( x->x_RFromValue + (x->x_RSmoothFactor * (x->x_fadeSamps - x->x_RSampsToFade) ) );
			--x->x_RSampsToFade;
		}
		
		if (x->x_LSampsToFade > 0) {
			
			x->x_LValue = ( x->x_LFromValue + (x->x_LSmoothFactor * (x->x_fadeSamps - x->x_LSampsToFade) ) );
			--x->x_LSampsToFade;
		}
		 
		*outL = ( (*inL * x->x_LValue) + (*inR * x->x_RValue) );
		
		inL++;
		inR++;
	}
	
	if ((x->x_LSampsToFade <= 0) && (x->x_RSampsToFade <= 0)) {
		setFadeTime(x, FADETIME_INIT);
	}
	
out:
	
	return (w+7);
}		

void xfade_dsp(t_xfade *x, t_signal **sp, short *count)	// method called when dsp is turned on
{
	x->x_Lcon = count[0]; // Left connected ?
	x->x_Rcon = count[1]; // Right connected ?
	
	if (!count[0])
		dsp_add(offset_perform, 5, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, x, sp[0]->s_n);
		//dsp_add(offset_perform, 4, sp[1]->s_vec, sp[2]->s_vec, x, sp[0]->s_n);
	else if (!count[1])
		dsp_add(offset_perform, 5, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, x, sp[0]->s_n);
		//dsp_add(offset_perform, 4, sp[0]->s_vec, sp[2]->s_vec, x, sp[0]->s_n);
	else
		dsp_add(xfade2_perform, 6, &x->x_obj.z_disabled, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

