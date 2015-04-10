
// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

/**
like the vanilla "decide" object but with probability (right inlet).
*/

#include "ext.h"
#include "ext_obex.h"
#include "ep.tools.h"

typedef struct _decide 
{
	t_object	f_ob;
	double      m_percent;
	void        *m_out[1];
	
} t_decide;

t_class *decide_class;

void decide_assist(t_decide *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET)
    {
		switch (a)
        {
			case 0:
				sprintf(s, "(bang) Make A Decision");
				break;
			default:
				sprintf(s, "Percent Of Chance (Now : %f)", x->m_percent);
				break;
		}
	} 
	else
    {
		switch (a)
        {
			case 0:
				sprintf(s, "Randomly Generated 0 or 1");
				break;
			default:
				sprintf(s, "outlet %ld", a);
				break;
		}
	}
}

void decide_inletinfo(t_decide *x, void *b, long index, char *t)
{
    *t = (index == 1);
}

short decide_decideFunction(t_decide *x)
{
	double randValue = rand();
	
	if (randValue >= RAND_MAX / (100./x->m_percent) || x->m_percent == 0)
    {
		return 0;
	}
	else
    {
		return 1;
	}
}

void decide_bang(t_decide *x)
{
	outlet_int(x->m_out[0], EP_pdecide(x->m_percent));
}

void decide_ft1(t_decide *x, double f)
{
	x->m_percent = EP_dclip(f, 0, 100);
}

void *decide_new(t_symbol *s, long argc, t_atom *argv)
{
	t_decide *x = (t_decide *)object_alloc(decide_class);
	
    if (x)
    {
        floatin(x, 1);
        
        x->m_out[0] = outlet_new(x, NULL);
        
        x->m_percent = 50;
        
        if (argc && argv && (atom_gettype(argv) == A_LONG || atom_gettype(argv) == A_FLOAT))
        {
            x->m_percent = atom_getfloat(argv);
        }
        
        x->m_percent = EP_dclip(x->m_percent, 0, 100);
    }
	
	return (x);
}

int C74_EXPORT main(void)
{
    t_class *c;
    
    c = class_new("ep.decide", (method)decide_new, (method)NULL, (long)sizeof(t_decide), 0L, A_GIMME, 0);
    
    class_addmethod(c, (method)decide_bang,         "bang",					0);
    class_addmethod(c, (method)decide_ft1,          "ft1",		A_FLOAT,	0);
    class_addmethod(c, (method)decide_assist,		"assist",	A_CANT,		0);
    class_addmethod(c, (method)decide_inletinfo,	"inletinfo", A_CANT,	0);
    
    class_register(CLASS_BOX, c);
    decide_class = c;
    
    return 0;
}