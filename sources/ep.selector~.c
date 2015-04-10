
// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

/*
 ep.selector~ works like the standard selector~ object of MSP distribution but integrate a crosseFade interpolation fonction.
*/

#include "ext.h"
#include "z_dsp.h"
#include "ext_obex.h"

#define MAX_CHANS (255)
#define PIOVER2 1.57079632679489661923132169163975144

typedef struct _selector
{
	t_pxobject x_obj;

	short   active_chan;
	short   last_chan;
	int     samps_to_fade;
	int     fadesamps;
	double  ramptime;
	int     number_of_channels;
    
} t_selector;

t_class *selector_class;

void selector_assist(t_selector *x, void *b, long msg, long a, char *s)
{
	if (msg==1)
    {
		if(a == 0)
        {
			sprintf(s,"(int/signal) Turns Input Off or Routes to Outputs");
		}
        else
        {
            sprintf(s,"(signal) Input %ld (%s)", a, (a == x->active_chan) ? "Open" : "Closed");
		}
		
	}
    else if (msg==2)
    {
		sprintf(s,"(signal) Output");
	}
}

void *selector_new(t_symbol *s, int argc, t_atom *argv)
{
	t_selector *x = (t_selector *)object_alloc(selector_class);
	
	if (x)
    {
        long active_channel = 0;
		x->ramptime = 100.;
		x->number_of_channels = 1;
        
		if(argc >= 1 && atom_gettype(argv) == A_LONG)
        {
            x->number_of_channels = atom_getlong(argv);
            if(x->number_of_channels < 2)
                x->number_of_channels = 2;
            if(x->number_of_channels > MAX_CHANS)
                x->number_of_channels = MAX_CHANS;
		}
        
		if(argc >= 2 && atom_gettype(argv+1) == A_LONG)
        {
            active_channel = atom_getlong(argv+1);
            if (active_channel < 0 || active_channel > x->number_of_channels - 1)
            {
                active_channel = 0;
            }
		}
        
        x->fadesamps = x->ramptime / 1000. * sys_getsr();
				
		dsp_setup((t_pxobject *)x, x->number_of_channels + 1);
		outlet_new((t_pxobject *)x, "signal");
		x->x_obj.z_misc |= Z_NO_INPLACE;
        
		x->active_chan = x->last_chan = active_channel;
		x->samps_to_fade = 0;
        
        attr_args_process(x, argc, argv);
	}
		
	return (x);
}

void selector_dsp_free(t_selector *x)
{
	dsp_free((t_pxobject *)x);
}

t_max_err selector_ramptime(t_selector *x, t_object *attr, long argc, t_atom *argv)
{
    if(argc && argv && (atom_gettype(argv) == A_LONG || atom_gettype(argv) == A_FLOAT))
    {
        double fade = atom_getfloat(argv);
        
        if(fade < 0.)
            fade = 0.;
        
        x->ramptime = fade;
        x->fadesamps = x->ramptime / 1000. * sys_getsr();
        x->samps_to_fade = 0;
    }
    
    return MAX_ERR_NONE;
}

void selector_channel(t_selector *x, long chan)
{
    if(chan != x->active_chan)
    {
        x->last_chan = x->active_chan;
        x->active_chan = chan;
        
        if(x->active_chan < 0)
        {
            x->active_chan = 0;
        }
        if(x->active_chan > x->number_of_channels)
        {
            x->active_chan = x->number_of_channels;
        }
        
        x->samps_to_fade = x->fadesamps;
    }
}

void selector_int(t_selector *x, t_int i)
{
    if (proxy_getinlet((t_object*)x) == 0)
    {
        selector_channel(x, (double)i);
    }
}

void selector_float(t_selector *x, t_float f)
{
    selector_int(x,(t_int)f);
}

void selector_perform64(t_selector *x, t_object *dsp64, double **ins, long nins, double **outs, long nouts, long vs, long f, void *p)
{
    if (x->active_chan > 0)
    {
        for(int i = 0; i < vs; i++)
        {
            if (x->samps_to_fade > 0)
            {
                const double phase = PIOVER2 * (1.0 - (x->samps_to_fade / (double)x->fadesamps));
                outs[0][i] = (ins[x->active_chan][i] * sin(phase)) + (ins[x->last_chan][i] * cos(phase));
                --x->samps_to_fade;
            }
            else
            {
                outs[0][i] = ins[x->active_chan][i];
            }
        }
    }
    else
    {
        for(int i = 0; i < vs; i++)
        {
            outs[0][i] = 0.;
        }
    }
}

void selector_perform64_signalrate(t_selector *x, t_object *dsp64, double **ins, long nins, double **outs, long nouts, long vs, long f, void *p)
{
    for(int i = 0; i < vs; i++)
    {
        const long chan = ins[0][i];
        
        if (chan >= 1 && chan <= x->number_of_channels)
        {
            outs[0][i] = ins[chan][i];
        }
        else
        {
            outs[0][i] = 0.;
        }
    }
}

void selector_dsp64(t_selector *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    x->fadesamps = x->ramptime / 1000. * sys_getsr();
    x->samps_to_fade = 0;
    
    if (count[0])
    {
        dsp_add64(dsp64, (t_object*)x, (t_perfroutine64)selector_perform64_signalrate, 0, NULL);
    }
    else
    {
        dsp_add64(dsp64, (t_object*)x, (t_perfroutine64)selector_perform64, 0, NULL);
    }
}

int C74_EXPORT main(void)
{
    t_class *c;
    
    c = class_new("ep.selector~", (method)selector_new, (method)dsp_free, (short)sizeof(t_selector), 0L, A_GIMME, 0);
    
    class_addmethod(c, (method)selector_dsp64,		"dsp64",	A_CANT, 0);
    class_addmethod(c, (method)selector_assist,		"assist",	A_CANT, 0);
    class_addmethod(c, (method)selector_channel,	"channel",	A_LONG, 0);
    class_addmethod(c, (method)selector_float,		"float",	A_FLOAT, 0);
    class_addmethod(c, (method)selector_int,		"int",		A_LONG, 0);
    
    CLASS_ATTR_DOUBLE           (c, "ramp", 0, t_selector, ramptime);
    CLASS_ATTR_CATEGORY			(c, "ramp", 0, "Behavior");
    CLASS_ATTR_LABEL			(c, "ramp", 0, "Ramp Time (ms)");
    CLASS_ATTR_ACCESSORS		(c, "ramp", NULL, selector_ramptime);
    CLASS_ATTR_ORDER			(c, "ramp", 0, "1");
    // @description The ramp time in milliseconds.
    
    class_dspinit(c);
    class_register(CLASS_BOX, c);
    selector_class = c;
    
    post("ep.selector~ object by Eliott Paris");
    
    return 0;
}