
// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

/**
multisplit object
*/

#include "ext.h"
#include "ext_obex.h"
#include "ep.tools.h"

typedef struct _multisplit 
{
	t_object	f_ob;
    long        m_numIn;
    long        m_numOut;
	t_atom      m_argv[128];
	long        m_number_of_arguments;
	void        *m_proxy[10];
	void        *m_out[128];
    
} t_multisplit;

t_class *multisplit_class;

void multisplit_assist(t_multisplit *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET)
    {
		sprintf(s, "Int/Float to be Split");
	} 
	else
    {
		if (a == x->m_numOut)
        {
			sprintf(s, "Output if Not Within Limits"); 
		}
		else
        {
			if (((x->m_argv + a)->a_type == A_LONG) && ((x->m_argv + a + 1)->a_type == A_LONG))
            {
				sprintf(s, "Output if between %ld and %ld", (long)atom_getlong(x->m_argv + a), (long)atom_getlong(x->m_argv + a + 1) );
			}
			if (((x->m_argv + a)->a_type == A_FLOAT) && ((x->m_argv + a + 1)->a_type == A_LONG))
            {
				sprintf(s, "Output if between %f and %ld", atom_getfloat(x->m_argv + a), (long)atom_getlong(x->m_argv + a + 1) );
			}
			if (((x->m_argv + a)->a_type == A_LONG) && ((x->m_argv + a + 1)->a_type == A_FLOAT))
            {
				sprintf(s, "Output if between %ld and %f", (long)atom_getlong(x->m_argv + a), atom_getfloat(x->m_argv + a + 1) );
			}
			if (((x->m_argv + a)->a_type == A_FLOAT) && ((x->m_argv + a + 1)->a_type == A_FLOAT))
            {
				sprintf(s, "Output if between %f and %f", atom_getfloat(x->m_argv + a), atom_getfloat(x->m_argv + a + 1) );
			}
		}
	}
}

void multisplit_inletinfo(t_multisplit *x, void *b, long index, char *t)
{
	*t = (index > 0);
}

void multisplit_free(t_multisplit *x)
{
	object_free(x->m_proxy);
}

void multisplit_set_boundaries(t_multisplit *x, t_symbol *s, long ac, t_atom *av)
{
	long inlet = proxy_getinlet((t_object *)x);
	if (ac > x->m_number_of_arguments)
    {
		ac = x->m_number_of_arguments;
	}
	
	if (inlet == 0)
    {
		for (int i = 0; i < ac; i++)
        {
			if ((av + i)->a_type == A_LONG)
            {
				atom_setlong(&x->m_argv[i], atom_getlong(av + i));
			}
			else if ((av + i)->a_type == A_FLOAT)
            {
				atom_setfloat(&x->m_argv[i], atom_getfloat(av + i));
			}
		}
	}
}

int floatBetween(double Value1, double Value2, double Current)
{
	if ((Value1 <= Current && Current <= Value2) || (Value2 <= Current && Current <= Value1))
    {
		return 1;
	}
	else
    {
		return 0;
	}
}

int intBetween(long Value1, long Value2, long Current)
{
    return floatBetween(Value1, Value2, Current);
}

void multisplit_int(t_multisplit *x, long n)
{
	long match = 0;
	long inlet = proxy_getinlet((t_object *)x);
	
	if (inlet == 0 && x->m_number_of_arguments >= 2)
    {
		for (int i = x->m_number_of_arguments-2; i >= 0; i--)
        {
			if (atom_isNumber(x->m_argv + i + 1) && atom_isNumber(x->m_argv + i))
            {
				if (floatBetween(atom_getfloat(x->m_argv + i + 1), atom_getfloat(x->m_argv + i), n) == 1 )
                {
					outlet_int(x->m_out[i], n);
					match = 1;
				}
			}
		}
				
		if (!match)
        {
			outlet_int(x->m_out[x->m_number_of_arguments - 1], n);
		}
	}
	
	if (inlet == 0 && x->m_number_of_arguments < 2)
    {
		if (atom_isNumber(x->m_argv + 1))
        {
			if (floatBetween(0, atom_getlong(x->m_argv + 1), n) ){
				
				outlet_int(x->m_out[0], n);
				
				match = 1;
			}
		}
		
		if (!match)
        {
			outlet_int(x->m_out[1], n);
		}
	}
}

////////////////////////////

void multisplit_float(t_multisplit *x, double f)
{
	long match = 0;
	long inlet = proxy_getinlet((t_object *)x);
	
	if (inlet == 0 && x->m_number_of_arguments >= 2)
    {
		for (int i = x->m_number_of_arguments -2; i >= 0; i--)
        {
			if (atom_isNumber(x->m_argv + i + 1) && atom_isNumber(x->m_argv + i))
            {
				if (floatBetween(atom_getfloat(x->m_argv + i + 1), atom_getfloat(x->m_argv + i), f) == 1 ){
					
					outlet_float(x->m_out[i], f);
					match = 1;
				}
			}
		}
		
		if (!match)
        {
			outlet_float(x->m_out[x->m_number_of_arguments - 1], f);
		}
	}
	
	if (inlet == 0 && x->m_number_of_arguments < 2)
    {
		if (atom_isNumber(x->m_argv + 1))
        {
			if (floatBetween(0, atom_getfloat(x->m_argv + 1), f))
            {
				outlet_float(x->m_out[0], f);
				match = 1;
			}
		}
        
		if (!match)
        {
			outlet_float(x->m_out[1], f);
		}
	}

}

void multisplit_list(t_multisplit *x, t_symbol *s, long ac, t_atom *av)
{
	long i = 0;
	long inlet = proxy_getinlet((t_object *)x);
	
	if (inlet == 0)
    {
		for (i = 0; i < ac; i++)
        {
			if (atom_gettype(av + i) == A_LONG)
            {
				multisplit_int(x, atom_getlong(av + i));
			}
            else if (atom_gettype(av + i) == A_FLOAT)
            {
                multisplit_float(x, atom_getfloat(av + i));
            }
		}
	}
	
}


/////////////////////////////////////////////////////////////////////////////////////////

void *multisplit_new(t_symbol *s, long argc, t_atom *argv)
{
	t_multisplit *x = (t_multisplit *)object_alloc(multisplit_class);
	
    if (x)
    {
        x->m_numOut = argc-1;
        x->m_number_of_arguments = argc;

        switch (argc)
        {
            case 0:
            {
                x->m_out[1] = outlet_new(x, NULL);
                x->m_out[0] = outlet_new(x, NULL);
                
                x->m_numOut = 1;
                atom_setlong(&x->m_argv[0], 0);
                atom_setfloat(&x->m_argv[1], 1.);
                x->m_number_of_arguments = 2;
                break;
            }
            case 1:
            {
                if (atom_isNumber(argv))
                {
                    x->m_out[1] = outlet_new(x, NULL);
                    x->m_out[0] = outlet_new(x, NULL);
                    
                    x->m_numOut = 1;
                    
                    atom_setlong(&x->m_argv[0], 0);
                    
                    if (atom_gettype(argv) == A_LONG)
                    {
                        atom_setlong(&x->m_argv[1], atom_getlong(argv));
                        x->m_number_of_arguments = 2;
                    }
                    else if (atom_gettype(argv) == A_FLOAT)
                    {
                        atom_setfloat(&x->m_argv[1], atom_getfloat(argv));
                        x->m_number_of_arguments = 2;
                    }
                }
                else
                {
                    object_error((t_object *)x, "args must be int or float");
                    return (0);
                }
                break;
            }
            default:
            {
                for (int i = argc-1; i >= 0; i--)
                {
                    if (atom_isNumber(argv + i))
                    {
                        x->m_out[i] = outlet_new(x, NULL);
                    }
                    else
                    {
                        object_error((t_object *)x, "args must be int or float");
                        return (0);
                    }
                }
                
                for (int i = 0; i < argc; i++)
                {
                    if (atom_gettype(argv + i) == A_LONG)
                    {
                        atom_setlong(&x->m_argv[i], atom_getlong(argv + i));
                    }
                    else if (atom_gettype(argv + i) == A_FLOAT)
                    {
                        atom_setfloat(&x->m_argv[i], atom_getfloat(argv + i));
                    }
                }
                
                break;
            }
        }
    }

	return (x);
}

int C74_EXPORT main(void)
{
    t_class *c;
    
    c = class_new("ep.multisplit", (method)multisplit_new, (method)multisplit_free, (long)sizeof(t_multisplit),
                  0L, A_GIMME, 0);
    
    class_addmethod(c, (method)multisplit_int,          "int",		A_LONG,		0);
    class_addmethod(c, (method)multisplit_float,        "float",	A_FLOAT,	0);
    class_addmethod(c, (method)multisplit_list,         "list",		A_GIMME,	0);
    class_addmethod(c, (method)multisplit_set_boundaries,	"set",	A_GIMME,	0);
    class_addmethod(c, (method)multisplit_assist,		"assist",	A_CANT,		0);
    
    class_register(CLASS_BOX, c);
    multisplit_class = c;
    
    return 0;
}