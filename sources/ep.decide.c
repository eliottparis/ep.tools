
// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

/**
like standard Max distrib decide Object but add probability function (right inlet).
*/

#include "ext.h"							// standard Max include, always required
#include "ext_obex.h"						// required for new style Max object

//////////////////////////

#include "EP_EXTERNS.h"
#define OBJECT_NAME "ep.decide"

////////////////////////// object struct
typedef struct _decide 
{
	t_object					ob;			// the object itself (must be first)
	double m_percent;
	void *m_out[1];
	
} t_decide;

///////////////////////// function prototypes

void decide_bang(t_decide *x);
void decide_int(t_decide *x, long n);
void decide_ft1(t_decide *x, double f);
//void decide_float(t_decide *x, double f);
//void decide_list(t_decide *x, t_symbol *s, long argc, t_atom *argv);
//void decide_anything(t_decide *x, t_symbol *s, long argc, t_atom *argv);

//// standard set
void *decide_new(t_symbol *s, long argc, t_atom *argv);
void decide_free(t_decide *x);
void decide_assist(t_decide *x, void *b, long m, long a, char *s);
void decide_inletinfo(t_decide *x, void *b, long index, char *t);

//// Object Specific functions :
short decide_decideFunction(t_decide *x);

//////////////////////// global class pointer variable
t_class *decide_class;


int C74_EXPORT main(void)
{	
	t_class *c;
	
	c = class_new("ep.decide", (method)decide_new, (method)decide_free, (long)sizeof(t_decide), 
				  0L /* leave NULL!! */, A_GIMME, 0);
	
	class_addmethod(c, (method)decide_bang,		"bang",					0);
	class_addmethod(c, (method)decide_int,		"int",		A_LONG,		0);
	class_addmethod(c, (method)decide_ft1,		"ft1",		A_FLOAT,	0);
	//class_addmethod(c, (method)decide_float,	"float",	A_FLOAT,	0);
	//class_addmethod(c, (method)decide_list,		"list",		A_GIMME,	0);
	//class_addmethod(c, (method)decide_anything,	"anything", A_GIMME,	0);
	
	/* you CAN'T call this from the patcher */
    class_addmethod(c, (method)decide_assist,		"assist",	A_CANT,		0);
	class_addmethod(c, (method)decide_inletinfo,	"inletinfo", A_CANT,	0);
	// si tout les inlets sont cold remplacer par :
	//class_addmethod(c, (method)stdinletinfo, "inletinfo", A_CANT, 0);
	
	class_register(CLASS_BOX, c); /* CLASS_NOBOX */
	decide_class = c;

	return 0;
}

void decide_assist(t_decide *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) { // inlet
		switch (a) {
			case 0:
				sprintf(s, "(bang) Make A Decision");
				break;
			default:
				sprintf(s, "Percent Of Chance (Now : %f)", x->m_percent);
				break;
		}
	} 
	else {	// outlet
		switch (a) {
			case 0:
				sprintf(s, "Randomly Generated 0 or 1");
				break;
			default:
				sprintf(s, "outlet %ld", a);
				break;
		}
	}
}

void decide_inletinfo(t_decide *x, void *b, long index, char *t) // to show cold inlet
{
	if (index  == 1)
		*t = 1;			// second inlet froid.
}

void decide_free(t_decide *x)
{
	;
}

/////////////////////////////////////////////////////////////////////////////////////////

short decide_decideFunction(t_decide *x)
{
	double randValue = rand();
	
	if (randValue >= RAND_MAX/(100/x->m_percent) || x->m_percent == 0) {
		return 0;
	}
	else {
		return 1;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void decide_bang(t_decide *x)
{
	outlet_int(x->m_out[0], EP_pdecide(x->m_percent));
	//outlet_int(x->m_out[0], decide_decideFunction(x));
}


////////////////////////////

void decide_int(t_decide *x, long n)
{	
	;
}

////////////////////////////

void decide_ft1(t_decide *x, double f)
{
	x->m_percent = EP_dclip(f, 0, 100);
}

/////////////////////////////////////////////////////////////////////////////////////////

void *decide_new(t_symbol *s, long argc, t_atom *argv)
{
	t_decide *x = NULL;
	
	x = (t_decide *)object_alloc(decide_class);
	
	floatin(x, 1);										// percent inlet creation
	
	x->m_out[0] = outlet_new(x, NULL);					// outlet generique.
	
	switch ((argv->a_type)) {
		case A_LONG:
			x->m_percent = atom_getlong(argv);
			break;
		case A_FLOAT:
			x->m_percent = atom_getfloat(argv);
			break;
		default:
			x->m_percent = 50;
			break;
	}
	x->m_percent = EP_dclip(x->m_percent, 0, 100);		// on s'assure que percent soit entre 0. et 100.
	
	return (x);
}
