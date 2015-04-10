
// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

/*
 >< is the same as the "==" object but with a tolerance value
 
 if ($f1 <= $f2 && $f1 >= ($f2 - $f3)) || ($f1 >= $f2 && $f1 <= ($f2 + $f3)) then 1 else 0
 
 $f1 = left / $f2 = right value / $f3 : tolerance
*/

#include "ext.h"
#include "ext_obex.h"

typedef struct _inside
{
	t_object    f_ob;
	double      f_tolerance;
	double      f_left;
	double      f_right;
	void        *f_outlet;
    
} t_inside;

t_class *inside_class;

void *inside_new(double right, double tolerance)
{
	t_inside *x = (t_inside *)object_alloc(inside_class);
    
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

void inside_assist(t_inside *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_OUTLET)
    {
        sprintf(s,"Result = [left > (right - + tolerance) < left]");
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

void inside_setTolerance(t_inside *x, double tol)
{
	if (tol >= 0)
    {
		x->f_tolerance = tol;
	}
	else
    {
		x->f_tolerance = tol * -1;
	}
}

//--------------------------------------------

void inside_outifinside(t_inside *x)
{
    if (((x->f_right <= x->f_left) && (x->f_right >= (x->f_left - x->f_tolerance))) ||
        ((x->f_right >= x->f_left) && (x->f_right <= (x->f_left + x->f_tolerance))))
    {
        outlet_int(x->f_outlet, 1);
    }
    else
    {
        outlet_int(x->f_outlet, 0);
    }
}

void inside_int(t_inside *x, long left)
{
	x->f_left = left;
	inside_outifinside(x);
}


void inside_float(t_inside *x, double left)
{
	x->f_left = left;
	inside_outifinside(x);
}

void inside_in1(t_inside *x, double right)
{	
	x->f_right = right;
}

void inside_bang(t_inside *x)
{
	inside_outifinside(x);
}

int C74_EXPORT main(void)
{
    t_class *c;
    
    c = class_new("><", (method)inside_new, (method)NULL, sizeof(t_inside), 0L, A_DEFFLOAT, A_DEFFLOAT, 0);
    
    class_addmethod(c, (method)inside_assist,       "assist",	A_CANT, 0);
    class_addmethod(c, (method)inside_bang,         "bang",		0);
    class_addmethod(c, (method)inside_int,          "int",		A_LONG, 0);
    class_addmethod(c, (method)inside_float,		"float",	A_FLOAT, 0);
    class_addmethod(c, (method)inside_in1,          "ft1",		A_FLOAT, 0);
    class_addmethod(c, (method)inside_setTolerance, "tolerance",A_FLOAT, 0);
    
    class_register(CLASS_BOX, c);
    inside_class = c;
    
    post("\"><\" object by Eliott Paris");
    return 0;
}

