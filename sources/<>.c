
// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

/*
 <> is the same as the "!=" object but with a tolerance value

 if ($f1 <= $f2 && $f1 >= ($f2 - $f3)) || ($f1 >= $f2 && $f1 <= ($f2 + $f3)) then 0 else 1
 
 $f1 = left / $f2 = right value / $f3 : tolerance
*/

#include "ext.h"
#include "ext_obex.h"

typedef struct _outside
{
	t_object    f_ob;
	double      f_tolerance;
	double      f_left;
	double      f_right;
	void        *f_outlet;
    
} t_outside;

t_class *outside_class;

void *outside_new(double right, double tolerance)
{
	t_outside *x = (t_outside *)object_alloc(outside_class);
	
    if (x)
    {
        floatin(x, 1);
        x->f_outlet = intout(x);
        
        x->f_left	= 0.;
        x->f_right	= right;
        x->f_tolerance = tolerance;
    }
	
	return(x);
}

void outside_assist(t_outside *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_OUTLET)
    {
		sprintf(s,"Result = [(right - tolerance) < left > (right + tolerance)]");
    }
	else
    {
		switch (a)
        {
		case 0:
			sprintf(s,"Set right operand, Trigger the calculation");
			break;
		case 1:
			sprintf(s,"Set right operand");
			break;
		}
	}
}

void outside_outifoutside(t_outside *x)
{
    if (((x->f_right <= x->f_left) && (x->f_right >= (x->f_left - x->f_tolerance))) ||
        ((x->f_right >= x->f_left) && (x->f_right <= (x->f_left + x->f_tolerance))))
    {
        
        outlet_int(x->f_outlet, 0);
    }
    else
    {
        outlet_int(x->f_outlet, 1);
    }
}

void outside_setTolerance(t_outside *x, double tol)
{
	if (tol >= 0) {
		x->f_tolerance = tol;
	}
	else {
		x->f_tolerance = tol * -1;
	}
}

void outside_int(t_outside *x, long left)
{
	x->f_left = left;
	outside_outifoutside(x);
}

void outside_float(t_outside *x, double left)
{
	x->f_left = left;
	outside_outifoutside(x);
}


void outside_in1(t_outside *x, double right)
{	
	x->f_right = right;
}

void outside_bang(t_outside *x)
{
	outside_outifoutside(x);
}

int C74_EXPORT main(void)
{
    t_class *c;
    
    c = class_new("<>", (method)outside_new, (method)NULL, sizeof(t_outside), 0L, A_DEFFLOAT, A_DEFFLOAT, 0);
    
    class_addmethod(c, (method)outside_assist,          "assist",	A_CANT, 0);
    class_addmethod(c, (method)outside_bang,            "bang",		0);
    class_addmethod(c, (method)outside_int,             "int",		A_LONG, 0);
    class_addmethod(c, (method)outside_float,           "float",	A_FLOAT, 0);
    class_addmethod(c, (method)outside_in1,             "ft1",		A_FLOAT, 0);
    class_addmethod(c, (method)outside_setTolerance,	"tolerance",A_FLOAT, 0);
    
    class_register(CLASS_BOX, c);
    outside_class = c;
    
    post("\"><\" object by Eliott Paris");
    return 0;
}
