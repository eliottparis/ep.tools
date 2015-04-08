
// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

/**
Randout	
*/

#include "ext.h"							// standard Max include, always required
#include "ext_obex.h"						// required for new style Max object
#include "stdlib.h"

////////////////////////// object struct
typedef struct _Randout 
{
	t_object					ob;			// the object itself (must be first)
	long m_numIn, m_numOut;					// nombre d'inlet, et d'outlet.
	void *m_proxy[4];							// proxy.
	void *m_out[4];							// outlet 1 = x->m_out[0], outlet 2 = x->m_out[1]... (prkoi [4] ? je ne sais pas !!)
	void *out0;								// outlet 1.
} t_Randout;

///////////////////////// function prototypes

void Randout_bang(t_Randout *x);
void Randout_int(t_Randout *x, long n);
void Randout_float(t_Randout *x, double f);
void Randout_list(t_Randout *x, t_symbol *s, long argc, t_atom *argv);
void Randout_anything(t_Randout *x, t_symbol *s, long argc, t_atom *argv);

//// standard set
void *Randout_new(t_symbol *s, long argc, t_atom *argv);
void Randout_free(t_Randout *x);
void Randout_assist(t_Randout *x, void *b, long m, long a, char *s);

//////////////////////// global class pointer variable
void *Randout_class;


int C74_EXPORT main(void)
{	
	t_class *c;
	
	c = class_new("ep.Randout", (method)Randout_new, (method)Randout_free, (long)sizeof(t_Randout), 
				  0L /* leave NULL!! */, A_GIMME, 0);
	
	class_addmethod(c, (method)Randout_bang,		"bang",					0);
	class_addmethod(c, (method)Randout_int,		"int",		A_LONG,		0);
	class_addmethod(c, (method)Randout_float,		"float",	A_FLOAT,	0);
	class_addmethod(c, (method)Randout_list,		"list",		A_GIMME,	0);
	class_addmethod(c, (method)Randout_anything,	"anything", A_GIMME,	0);
	
	/* you CAN'T call this from the patcher */
    class_addmethod(c, (method)Randout_assist,		"assist",	A_CANT,		0);  
	
	class_register(CLASS_BOX, c); /* CLASS_NOBOX */
	Randout_class = c;

	post("Randout object "__DATE__"");
	return 0;
}

void Randout_assist(t_Randout *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) { // inlet
		sprintf(s, "msg in");
	} 
	else {	// outlet
		if (a == x->m_numOut) {
			sprintf(s, "Index of the last outlet opened"); 
		}
		else {
			sprintf(s, "outlet %ld", a); 
		}
	}
}

void Randout_inletinfo(t_Randout *x, void *b, long index, char *t) // to show cold inlet
{
	if (index > 0) *t = 1;		// inlet 1 froid.
}

void Randout_free(t_Randout *x)
{
	object_free(x->m_proxy);
}

/////////////////////////////////////////////////////////////////////////////////////////

void Randout_bang(t_Randout *x)
{
	//long inlet = proxy_getinlet((t_object *)x); // par quel inlet le message est transmis
	
	long randvalue;
	
	randvalue = rand()%x->m_numOut; 
	
	outlet_int(x->m_out[x->m_numOut], randvalue);
	
	outlet_bang(x->m_out[randvalue]);			// sort un bang dans le dernier outlet.
	
	//post("bang input in inlet %ld", inlet);
}

////////////////////////////

void Randout_int(t_Randout *x, long n)
{
	//long inlet = proxy_getinlet((t_object *)x); // par quel inlet le message est transmis
	
	long randvalue;
	
	randvalue = rand()%x->m_numOut; 
	
	outlet_int(x->m_out[x->m_numOut], randvalue);
	
	outlet_int(x->m_out[randvalue], n);
	
	//post("int input in inlet %ld", inlet);
}

////////////////////////////

void Randout_float(t_Randout *x, double f)
{
	//long inlet = proxy_getinlet((t_object *)x); // par quel inlet le message est transmis
	
	long randvalue;
	
	randvalue = rand()%x->m_numOut; 
	
	outlet_int(x->m_out[x->m_numOut], randvalue);
	
	outlet_float(x->m_out[randvalue], f);
	
	//post("float input in inlet %ld", inlet);
}

////////////////////////////

void Randout_list(t_Randout *x, t_symbol *s, long argc, t_atom *argv)
{	
	//long inlet = proxy_getinlet((t_object *)x); // par quel inlet le message est transmis
	
	long randvalue;
	
	randvalue = rand()%x->m_numOut; 
	
	outlet_int(x->m_out[x->m_numOut], randvalue);
	
	outlet_list(x->m_out[randvalue], s, argc, argv);
	
	//post("list input in inlet %ld", inlet);
}

////////////////////////////

void Randout_anything(t_Randout *x, t_symbol *s, long argc, t_atom *argv)
{
	//long inlet = proxy_getinlet((t_object *)x); // par quel inlet le message est transmis
	
	long randvalue;
	
	randvalue = rand()%x->m_numOut; 
	
	outlet_int(x->m_out[x->m_numOut], randvalue);
	
	outlet_anything(x->m_out[randvalue], s, argc, argv);
	
	//post("anythin input in inlet %ld", inlet);
}



/////////////////////////////////////////////////////////////////////////////////////////

void *Randout_new(t_symbol *s, long argc, t_atom *argv)
{
	t_Randout *x = NULL;
	
    long i;
	
	x = (t_Randout *)object_alloc(Randout_class);
	
	if ((argv)->a_type == A_LONG) {
		
		x->m_numOut = x->m_numIn = atom_getlong(argv);
	}
	else {
		
		x->m_numOut = x->m_numIn = 2;
	}
		
	for (i = x->m_numOut; i >= 0; i--){
		x->m_out[i] = outlet_new(x, NULL);					// Nbr d'outlet.
	}
		
	return (x);
}
