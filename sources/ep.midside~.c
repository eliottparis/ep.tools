
// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

/**
 Middle-Side Encoding object.
 Middle = (L + R) / 2
 Side = (L - R) / 2
*/

#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"

typedef struct _midside
{
    t_pxobject  f_obj;
	t_sample    f_middle_from_value;
	t_sample    f_side_from_value;
	t_sample    f_middle_to_value;
	t_sample    f_side_to_value;
	t_sample    f_middle_gain;
	t_sample    f_side_gain;
	t_sample    f_middle_smooth_factor;
	t_sample    f_side_smooth_factor;
	t_sample    f_fade_time;
	t_int       f_fade_samples;
	t_sample    f_middle_samples_to_fade;
	t_sample    f_side_samples_to_fade;
	
} t_midside;

t_class *midside_class;

void midside_assist(t_midside *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET)
    {
		switch (a)
        {
		case 0:
			sprintf(s,"(Signal) Left + middle/side setting");
			break;
		case 1:
			sprintf(s,"(Signal) Right");
			break;
		}
	} 
	else
    {
		switch (a)
        {
			case 0:
				sprintf(s,"(Signal) MS Encoded Left");
				break;
			case 1:
				sprintf(s,"(Signal) MS Encoded Right");
				break;
		}
	}
}

void samps_to_fade(t_midside *x)
{
	x->f_fade_samples = x->f_fade_time * sys_getsr();
}

void midside_set_middle(t_midside *x, double f)
{
	if (f >= 0)
    {
		x->f_middle_from_value = x->f_middle_gain;
		x->f_middle_to_value = f;
	}
	else
    {
		x->f_middle_from_value = x->f_middle_gain;
		x->f_middle_to_value = 0;
	}
	
	samps_to_fade(x);
	x->f_middle_smooth_factor = ((x->f_middle_to_value - x->f_middle_from_value) / x->f_fade_samples );
	x->f_middle_samples_to_fade = x->f_fade_samples;
}

void midside_set_side(t_midside *x, double f)
{
	if (f >= 0)
    {
		x->f_side_from_value = x->f_side_gain;
		x->f_side_to_value = f;
	}
	else
    {
		x->f_side_from_value = x->f_side_gain;
		x->f_side_to_value = 0;
	}
	
	samps_to_fade(x);
	x->f_side_smooth_factor = ((x->f_side_to_value - x->f_side_from_value) / x->f_fade_samples );
	x->f_side_samples_to_fade = x->f_fade_samples;
}

void midside_perform64_zero(t_midside *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    for(int i = 0; i < sampleframes; i++)
    {
        outs[0][i] = outs[1][i] = 0.;
    }
}

void midside_perform64_left(t_midside *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    for(int i = 0; i < sampleframes; i++)
    {
         outs[0][i] = outs[1][i] = ins[0][i];
    }
}

void midside_perform64_right(t_midside *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    for(int i = 0; i < sampleframes; i++)
    {
        outs[0][i] = outs[1][i] = ins[1][i];
    }
}

void midside_perform64(t_midside *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    for(int i = 0; i < sampleframes; i++)
    {
        if (x->f_side_samples_to_fade > 0)
        {
            x->f_side_gain = (x->f_side_from_value + (x->f_side_smooth_factor * (x->f_fade_samples - x->f_side_samples_to_fade)));
            --x->f_side_samples_to_fade;
        }
        
        if (x->f_middle_samples_to_fade > 0)
        {
            x->f_middle_gain = (x->f_middle_from_value + (x->f_middle_smooth_factor * (x->f_fade_samples - x->f_middle_samples_to_fade)));
            --x->f_middle_samples_to_fade;
        }
        
        const double side = (((ins[0][i] - ins[1][i]) / 2.) * x->f_side_gain);
        const double mid  = (((ins[0][i] + ins[1][i]) / 2.) * x->f_middle_gain);
        
        outs[0][i] = mid + side;
        outs[1][i] = mid - side;
    }
}

void midside_dsp64(t_midside *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    if (count[0] && count[1])
    {
        dsp_add64(dsp64, (t_object*)x, (t_perfroutine64)midside_perform64, 0, NULL);
    }
    else if(count[0])
    {
        dsp_add64(dsp64, (t_object*)x, (t_perfroutine64)midside_perform64_left, 0, NULL);
    }
    else if(count[1])
    {
        dsp_add64(dsp64, (t_object*)x, (t_perfroutine64)midside_perform64_right, 0, NULL);
    }
    else
    {
        dsp_add64(dsp64, (t_object*)x, (t_perfroutine64)midside_perform64_zero, 0, NULL);
    }
}

void *midside_new(t_symbol *s, long argc, t_atom *argv)
{
    t_midside *x = (t_midside *)object_alloc(midside_class);
    dsp_setup((t_pxobject *)x,2);
    outlet_new((t_pxobject *)x, "signal");
    outlet_new((t_pxobject *)x, "signal");
    
    x->f_fade_time = 5. / 1000.;
    x->f_fade_samples = x->f_fade_time * sys_getsr();
    x->f_middle_gain = x->f_side_gain = 0.5;
    
    return (x);
}

int C74_EXPORT main(void)
{
    t_class *c;
    
    c = class_new("ep.midside~", (method)midside_new, (method)dsp_free, (short)sizeof(t_midside), 0L, A_GIMME, 0);
    
    class_addmethod(c, (method)midside_dsp64,       "dsp64",    A_CANT,     0);
    class_addmethod(c, (method)midside_set_middle,  "middle",   A_FLOAT,    0);
    class_addmethod(c, (method)midside_set_side,    "side",     A_FLOAT,    0);
    class_addmethod(c, (method)midside_assist,      "assist",   A_CANT,     0);
    class_dspinit(c);
    class_register(CLASS_BOX, c);
    midside_class = c;
    
    post("midside~ object by Eliott Paris");
    
    return 0;
}
