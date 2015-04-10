
// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

/*
catch attend de rattraper la valeur stockee avant de coller a la valeur entrante.
*/

#include "ext.h"
#include "ext_obex.h"

typedef struct _catch
{
	t_object    f_ob;
	long        f_flag;
	long        f_match;
	long        f_lockFunc;
	double      f_tolerance;
	double      f_store;
	double      f_current;
	void        *f_outlet;
	void        *f_outlet1;
	void        *f_outlet2;
    
} t_catch;

t_class *catch_class;

void *catch_new(double tolerance, double stored)
{
	t_catch *x = (t_catch *)object_alloc(catch_class);
	
    if (x)
    {
        intin(x,1);
        
        x->f_outlet2 = intout(x);
        x->f_outlet1 = floatout(x);
        x->f_outlet  = floatout(x);
        
        x->f_current	= 0.;
        x->f_store	= stored;
        x->f_flag	= 0;
        x->f_lockFunc	= 0;
        x->f_match	= 1;
        x->f_tolerance = tolerance;
    }
    
    return(x);
}

void catch_assist(t_catch *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_OUTLET)
    {
		switch (a)
        {
		case 0:
            sprintf(s,"catch value (%s)", (x->f_flag == 1) ? "Open" : "Closed");
			break;
		case 1:
			sprintf(s,"Pass through right inlet value");
			break;
		case 2:
			sprintf(s,"Output 1 if catch, 0 if uncatch");
			break;
		}
    }
	else
    {
		switch (a)
        {
		case 0:
			sprintf(s,"(float) value in");
			break;
		case 1:
			sprintf(s,"toggle on/off");
			break;
		}
	}
}

void catch_outifCatch(t_catch *x, double f)
{
    x->f_match = (x->f_store == x->f_current);
    
    if (x->f_match == 0 && x->f_lockFunc == 0)
    {
        if (((x->f_current <= x->f_store) && (x->f_current >= (x->f_store - x->f_tolerance))) ||
            ((x->f_current >= x->f_store) && (x->f_current <= (x->f_store + x->f_tolerance))))
        {
            outlet_int(x->f_outlet2, 1);
            outlet_float(x->f_outlet, f);
            x->f_store = x->f_current;
            x->f_match	= 1;
            x->f_lockFunc = 1;
        }
        else
        {
            x->f_match = 0;
        }
    }
    else
    {
        outlet_int(x->f_outlet2, 1);
        outlet_float(x->f_outlet, f);
        x->f_store = x->f_current;
        x->f_match	= 1;
        x->f_lockFunc = 1;
    }
}

void catch_setTolerance(t_catch *x, double tolerance)
{
	if(tolerance >= 0)
    {
		x->f_tolerance = tolerance;
	}
	else
    {
		x->f_tolerance = tolerance * -1;
	}
}

void catch_setStore(t_catch *x, double sto)
{
	x->f_store = sto;
	x->f_lockFunc = 0;
}

void catch_float(t_catch *x, double f)
{
	outlet_float(x->f_outlet1, f);
	x->f_current = f;
	
	if (x->f_flag == 1)
    {
		catch_outifCatch(x, f);
	}
	else if (x->f_store == x->f_current)
    {
        x->f_match = 1;
    }
    else
    {
        x->f_match = 0;
    }
    
	if (x->f_flag == 1 && x->f_match == 0)
    {
		outlet_int(x->f_outlet2, 0);
	}
}

void catch_int(t_catch *x, long n)
{
    catch_float(x, n);
}

void catch_in1(t_catch *x, long n)
{	
	if (n == 0)
    {
		x->f_flag = 0;
		outlet_int(x->f_outlet2, 0);
		
		if (x->f_match == 1)
        {
			x->f_store = x->f_current;
		}
	}
	else
    {
		x->f_flag = 1;
        
		if (x->f_store != x->f_current)
        {
			x->f_lockFunc = 0;
		}
		else
        {
			x->f_lockFunc = 1;
		}
	}
}

void catch_bang(t_catch *x)
{
	outlet_float(x->f_outlet, x->f_store);
}

int C74_EXPORT main(void)
{
    t_class *c;
    
    c = class_new("ep.catch", (method)catch_new, (method)NULL, sizeof(t_catch), 0L, A_DEFFLOAT, A_DEFFLOAT, 0);
    
    class_addmethod(c, (method)catch_assist,        "assist",	A_CANT, 0);
    class_addmethod(c, (method)catch_bang,          "bang",		0);
    class_addmethod(c, (method)catch_int,           "int",		A_LONG, 0);
    class_addmethod(c, (method)catch_float,         "float",	A_FLOAT, 0);
    class_addmethod(c, (method)catch_in1,           "in1",		A_LONG, 0);
    class_addmethod(c, (method)catch_setTolerance,	"tolerance",A_FLOAT, 0);
    class_addmethod(c, (method)catch_setStore,		"set",		A_FLOAT, 0);
    
    class_register(CLASS_BOX, c);
    catch_class = c;
    
    post("catch object by Eliott Paris");
    return 0;
}

