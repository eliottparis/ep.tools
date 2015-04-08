
// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

/**

 simple MSP Object who deals with Middle-Side formula.
 
 Middle = (L + R) / 2
 Side = (L - R) / 2
 
 */

#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"

#include "../EP_EXTERNS.h"
#define OBJECT_NAME "ep.midside~"

void *midside_class;

typedef struct _midside
{
    t_pxobject x_obj;
	
	t_float x_midFromValue;
	t_float x_sideFromValue;
	t_float x_midToValue;
	t_float x_sideToValue;
	t_float x_midValue;
	t_float x_sideValue;
	t_float x_midSmoothFactor;
	t_float x_sideSmoothFactor;
	t_float x_fadeTime;
	t_int x_fadeSamps;
	t_float x_midSampsToFade;
	t_float x_sideSampsToFade;
	t_float x_sr;
	
	t_int x_Lcon; // Left connected ?
	t_int x_Rcon; // Right connected ?
	
} t_midside;

void *midside_new(double val);
t_int *offset_perform(t_int *w);
t_int *midside2_perform(t_int *w);

void midside_SetMiddle(t_midside *x, double f);
void midside_SetSide(t_midside *x, double f);
void samps_to_fade(t_midside *x);

void midside_float(t_midside *x, double f);
void midside_int(t_midside *x, long n);
void midside_dsp(t_midside *x, t_signal **sp, short *count);
void midside_assist(t_midside *x, void *b, long m, long a, char *s);

int main(void)
{
    t_class *c;

	c = class_new("ep.midside~", (method)midside_new, (method)dsp_free, (short)sizeof(t_midside), 0L, A_DEFFLOAT, 0);
    
    class_addmethod(c, (method)midside_dsp, "dsp", A_CANT, 0);				// respond to the dsp message 
																			// (sent to MSP objects when audio is turned on/off)
    class_addmethod(c, (method)midside_float, "float", A_FLOAT, 0);
    class_addmethod(c, (method)midside_int, "int", A_LONG, 0);
	class_addmethod(c, (method)midside_SetMiddle, "middle", A_FLOAT, 0);
	class_addmethod(c, (method)midside_SetSide, "side", A_FLOAT, 0);
	class_addmethod(c, (method)midside_assist,"assist",A_CANT,0);
    class_dspinit(c);														// must call this function for MSP object classes

	class_register(CLASS_BOX, c);
	midside_class = c;
	
	post("%s %s", OBJECT_NAME, EP_EXTERNS_MSG);
	
	return 0;
}

void midside_assist(t_midside *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) {
		switch (a) {	
		case 0:
			sprintf(s,"(Signal) Left + middle/side Setting");
			break;
		case 1:
			sprintf(s,"(Signal) Right");
			break;
		}
	} 
	else {
		switch (a) {	
			case 0:
				sprintf(s,"(Signal) MS Encoded Left");
				break;
			case 1:
				sprintf(s,"(Signal) MS Encoded Right");
				break;
		}
	}
}

void *midside_new(double val)
{
    t_midside *x = object_alloc(midside_class);
    dsp_setup((t_pxobject *)x,2);					// set up DSP for the instance and create 2 signal inlets
	outlet_new((t_pxobject *)x, "signal");
    outlet_new((t_pxobject *)x, "signal");			// signal outlets are created like this
	
	x->x_sr = sys_getsr();							// get sampling rate and store it
	if(!x->x_sr){
		x->x_sr = 44100.0;
		error("zero sampling rate - set to 44100");
	}
	
	x->x_fadeTime = 5. / 1000.;
	x->x_fadeSamps = x->x_fadeTime * x->x_sr;
	
	//post("sampling rate = %f, fadeSamps = %ld", x->x_sr, x->x_fadeSamps);
	
	x->x_midValue = x->x_sideValue = 0.5;
	
    return (x);
}

void samps_to_fade(t_midside *x)
{
	x->x_sr = sys_getsr();							// get sampling rate and store it
	if(!x->x_sr){
		x->x_sr = 44100.0;
		error("zero sampling rate - set to 44100");
	}
	
	x->x_fadeSamps = x->x_fadeTime * x->x_sr;
	
	//x->x_midSmoothFactor = ((x->x_midToValue - x->x_midFromValue) / x->x_fadeSamps );
	//x->x_sideSmoothFactor = ((x->x_sideToValue - x->x_sideFromValue) / x->x_fadeSamps );
	//x->x_sideSampsToFade = x->x_fadeSamps;
	//x->x_midSampsToFade = x->x_fadeSamps;
	//post("Factor = %f", x->x_midSmoothFactor);
}

void midside_SetMiddle(t_midside *x, double f)
{
	if (f >= 0) {
		x->x_midFromValue = x->x_midValue;
		x->x_midToValue = f;
		//x->x_midValue = f;
	}
	else {
		x->x_midFromValue = x->x_midValue;
		x->x_midToValue = 0;
		//x->x_midValue = 0;
	}
	
	samps_to_fade(x);
	x->x_midSmoothFactor = ((x->x_midToValue - x->x_midFromValue) / x->x_fadeSamps );
	x->x_midSampsToFade = x->x_fadeSamps;
}

void midside_SetSide(t_midside *x, double f)
{
	if (f >= 0) {
		x->x_sideFromValue = x->x_sideValue;
		x->x_sideToValue = f;
		//x->x_sideValue = f;
	}
	else {
		x->x_sideFromValue = x->x_sideValue;
		x->x_sideToValue = 0;
		//x->x_sideValue = 0
	}
	
	samps_to_fade(x);
	x->x_sideSmoothFactor = ((x->x_sideToValue - x->x_sideFromValue) / x->x_fadeSamps );
	x->x_sideSampsToFade = x->x_fadeSamps;
}

void midside_float(t_midside *x, double f)				// the float and int routines cover both inlets. 
{														// It doesn't matter which one is involved
	;
}

void midside_int(t_midside *x, long n)
{
	post("L connected : %ld, R connected : %ld", x->x_Lcon, x->x_Rcon);
}

t_int *offset_perform(t_int *w)							// our perform method if one signal inlet is connected
{														// a optimiser => signal mono dans outL qd gauche connecte, 0 signal si seulement droit..!
    t_float *inL = (t_float *)(w[1]);
	t_float *inR = (t_float *)(w[2]);
    t_float *outL = (t_float *)(w[3]);
	t_float *outR = (t_float *)(w[4]);
	t_midside *x = (t_midside *)(w[5]);
	int n = (int)(w[6]);
	long i;
	
	if (x->x_obj.z_disabled)
		goto out;
	
	else if (x->x_Lcon) {								// si seul le canal gauche est connecte => Mono L = R = inL
		for (i=0; i<n; i++, outL++, outR++) {
			*outL = *outR = *inL;
			inL++;
		}
	}
	else {												// si seul le canal droit est connecte => Mono L = R = inR
		for (i=0; i<n; i++, outL++, outR++) {
			*outL = *outR = *inR;
			inR++;
		}
	}
	
out:
    return (w+7);
}

t_int *midside2_perform(t_int *w)						// our perform method if both signal inlets are connected
{
	t_float *inL,*inR,*outL,*outR;
	int n;
	int i = 0;
	double MidSig;
	double SideSig;

	if (*(long *)(w[1]))
	    goto out;

	t_midside *x = (t_midside *)(w[2]);
	inL = (t_float *)(w[3]);
	inR = (t_float *)(w[4]);
	outL = (t_float *)(w[5]);
	outR = (t_float *)(w[6]);
	n = (int)(w[7]);
	
	for (i=0; i<n; i++, outL++, outR++) {
		
		if (x->x_sideSampsToFade > 0) {
			
			x->x_sideValue = ( x->x_sideFromValue + (x->x_sideSmoothFactor * (x->x_fadeSamps - x->x_sideSampsToFade) ) );
			--x->x_sideSampsToFade;
		}
		
		if (x->x_midSampsToFade > 0) {
			
			x->x_midValue = ( x->x_midFromValue + (x->x_midSmoothFactor * (x->x_fadeSamps - x->x_midSampsToFade) ) );
			--x->x_midSampsToFade;
		}
		
		SideSig = ( ( (*inL - *inR) / 2.) * x->x_sideValue);
		MidSig = ( ( (*inL + *inR) / 2.) * x->x_midValue);
		
		*outR = MidSig  -  SideSig;
		*outL = MidSig  +  SideSig;
		
		inL++;
		inR++;
	}
	
out:
	
	return (w+8);
}		

void midside_dsp(t_midside *x, t_signal **sp, short *count)	// method called when dsp is turned on
{
	x->x_Lcon = count[0]; // Left connected ?
	x->x_Rcon = count[1]; // Right connected ?
	
	if (!count[0])		// si aucun signal connecte a inlet 0
		dsp_add(offset_perform, 6, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, x, sp[0]->s_n);
		//dsp_add(offset_perform, 4, sp[1]->s_vec, sp[2]->s_vec, x, sp[0]->s_n);
	else if (!count[1])	// si aucun signal connecte a inlet 1
		dsp_add(offset_perform, 6, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, x, sp[0]->s_n);
		//dsp_add(offset_perform, 4, sp[0]->s_vec, sp[2]->s_vec, x, sp[0]->s_n);
	else
		dsp_add(midside2_perform, 7, &x->x_obj.z_disabled, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);
}

