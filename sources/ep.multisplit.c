
// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

/**
multisplit	
*/

#include "ext.h"							// standard Max include, always required
#include "ext_obex.h"						// required for new style Max object

////////////////////////// object struct
typedef struct _multisplit 
{
	t_object					ob;			// the object itself (must be first)
	long m_numIn, m_numOut;					// nombre d'inlet, et d'outlet.
	t_atom m_argv[128];						// liste des arguments passes a l'objet.
	long m_argLen;							// Nbr d'args passes a l'objet.
	void *m_proxy[10];						// proxy.
	void *m_out[128];						// outlet 1 = x->m_out[0], outlet 2 = x->m_out[1]... ([128] = Maximum out sinon bad object quand freed !!)
} t_multisplit;

///////////////////////// function prototypes

void multisplit_int(t_multisplit *x, long n);
void multisplit_float(t_multisplit *x, double f);
void multisplit_list(t_multisplit *x, t_symbol *s, long argc, t_atom *argv);

void multisplit_setBornes(t_multisplit *x, t_symbol *s, long ac, t_atom *av);  // Change les bornes des splits (remplace les args).

int intBetween(long Value1, long Value2, long Current);
int floatBetween(double Value1, double Value2, double Current);

//// standard set
void *multisplit_new(t_symbol *s, long argc, t_atom *argv);
void multisplit_free(t_multisplit *x);
void multisplit_assist(t_multisplit *x, void *b, long m, long a, char *s);

//////////////////////// global class pointer variable
void *multisplit_class;


int main(void)
{	
	t_class *c;
	
	c = class_new("ep.multisplit", (method)multisplit_new, (method)multisplit_free, (long)sizeof(t_multisplit), 
				  0L /* leave NULL!! */, A_GIMME, 0);
	
	class_addmethod(c, (method)multisplit_int,		"int",		A_LONG,		0);
	class_addmethod(c, (method)multisplit_float,	"float",	A_FLOAT,	0);
	class_addmethod(c, (method)multisplit_list,		"list",		A_GIMME,	0);
	
	class_addmethod(c, (method)multisplit_setBornes,	"set",	A_GIMME,	0);
	
	/* you CAN'T call this from the patcher */
    class_addmethod(c, (method)multisplit_assist,		"assist",	A_CANT,		0);  
	
	class_register(CLASS_BOX, c); /* CLASS_NOBOX */
	multisplit_class = c;

	post("multisplit object "__DATE__" by Eliott Paris");
	return 0;
}

void multisplit_assist(t_multisplit *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) { // inlet
		sprintf(s, "Int/Float to be Split");
	} 
	else {	// outlet
		if (a == x->m_numOut) {
			sprintf(s, "Output if Not Within Limits"); 
		}
		else {
			if (((x->m_argv + a)->a_type == A_LONG) && ((x->m_argv + a + 1)->a_type == A_LONG)) {
				sprintf(s, "Output if between %ld and %ld", atom_getlong(x->m_argv + a), atom_getlong(x->m_argv + a + 1) );
			}
			if (((x->m_argv + a)->a_type == A_FLOAT) && ((x->m_argv + a + 1)->a_type == A_LONG)) {
				sprintf(s, "Output if between %f and %ld", atom_getfloat(x->m_argv + a), atom_getlong(x->m_argv + a + 1) );
			}
			if (((x->m_argv + a)->a_type == A_LONG) && ((x->m_argv + a + 1)->a_type == A_FLOAT)) {
				sprintf(s, "Output if between %ld and %f", atom_getlong(x->m_argv + a), atom_getfloat(x->m_argv + a + 1) );
			}
			if (((x->m_argv + a)->a_type == A_FLOAT) && ((x->m_argv + a + 1)->a_type == A_FLOAT)) {
				sprintf(s, "Output if between %f and %f", atom_getfloat(x->m_argv + a), atom_getfloat(x->m_argv + a + 1) );
			}
			//sprintf(s, "Output if Within Limits %ld", a); 
		}
	}
}

void multisplit_inletinfo(t_multisplit *x, void *b, long index, char *t) // to show cold inlet
{
	if (index > 0) *t = 1;		// inlet 1 froid.
}

void multisplit_free(t_multisplit *x)
{
	object_free(x->m_proxy);
}

/////////////////////////////////////////////////////////////////////////////////////////

void multisplit_setBornes(t_multisplit *x, t_symbol *s, long ac, t_atom *av)
{
	long i = 0;
	long inlet = proxy_getinlet((t_object *)x);
	if (ac > x->m_argLen) {
		ac = x->m_argLen;
	}
	
	if (inlet == 0) {
		
		for (i = 0; i < ac; i++){
			if ((av + i)->a_type == A_LONG) {
				atom_setlong(&x->m_argv[i], atom_getlong(av + i));
				//post("enum args %ld = %ld", i, atom_getlong(x->m_argv + i));
			}
			else if ((av + i)->a_type == A_FLOAT) {
				atom_setfloat(&x->m_argv[i], atom_getfloat(av + i));
				//post("enum args %ld = %2f", i, atom_getfloat(x->m_argv + i));
			}
		}
	}
}

//////////////////////////

int intBetween(long Value1, long Value2, long Current)
{
	//post("intBetween called with : %ld %ld %ld", Value1, Value2, Current);
	
	if ( (Value1 <= Current && Current <= Value2) || (Value2 <= Current && Current <= Value1) ) {
		//post("intBetween retourne 1");
		return 1;
	}
	else {
		//post("intBetween retourne 0");
		return 0;
	}
	//return 0;
}

int floatBetween(double Value1, double Value2, double Current)
{
	if ( (Value1 <= Current && Current <= Value2) || (Value2 <= Current && Current <= Value1) ) {
		return 1;
	}
	else {
		return 0;
	}
	//return 0;
}

////////////////////////////////////////////////////////////////////////////////////////

void multisplit_int(t_multisplit *x, long n)
{
	long i = 0;
	long match = 0;
	
	long inlet = proxy_getinlet((t_object *)x);
	
	
	if (inlet == 0 && x->m_argLen >= 2) {
		
		
		for (i = x->m_argLen-2; i >= 0; i--) {	
			
			//post("boucle %ld, value in : %ld", i, n);
			//post("argv %ld : %ld", i, atom_getlong(x->m_argv + i));
			//post("argLen = %ld",x->m_argLen);
			
			
			if ( ((x->m_argv + i + 1)->a_type == A_LONG) && ((x->m_argv + i)->a_type == A_LONG) ){
								
				if ( intBetween(atom_getlong(x->m_argv + i + 1), atom_getlong(x->m_argv + i), n) == 1 ){
					
					outlet_int(x->m_out[i], n);
					
					match = 1;
				}
			}
			
				
			if ( ((x->m_argv + i + 1)->a_type == A_FLOAT) && ((x->m_argv + i)->a_type == A_LONG) ){
					
				if ( floatBetween(atom_getfloat(x->m_argv + i + 1), atom_getlong(x->m_argv + i), n) ){
						
					outlet_int(x->m_out[i], n);
					
					match = 1;
				}
			}
			
				
			if ( ((x->m_argv + i + 1)->a_type == A_LONG) && ((x->m_argv + i)->a_type == A_FLOAT) ){
					
				if ( floatBetween(atom_getlong(x->m_argv + i + 1), atom_getfloat(x->m_argv + i), n) ){
						
					outlet_int(x->m_out[i], n);
						
					match = 1;
				}
			}
			
			
			if ( ((x->m_argv + i + 1)->a_type == A_FLOAT) && ((x->m_argv + i)->a_type == A_FLOAT) ){
				
				if ( floatBetween(atom_getfloat(x->m_argv + i + 1), atom_getfloat(x->m_argv + i), n) ){
					
					outlet_int(x->m_out[i], n);
					
					match = 1;
				}
			}
		}
				
		if (!match) {
			outlet_int(x->m_out[x->m_argLen - 1], n);
		}
	}
	
	
	// si l'objet a moins de 2 args :
	
	if (inlet == 0 && x->m_argLen < 2) {
	
		if ( (x->m_argv + 1)->a_type == A_LONG){
			
			if ( intBetween(0, atom_getlong(x->m_argv + 1), n) ){
				
				outlet_int(x->m_out[0], n);
				
				match = 1;
			}
		}
		if ( (x->m_argv + 1)->a_type == A_FLOAT){
			
			if ( floatBetween(0, atom_getfloat(x->m_argv + 1), n) ){
				
				outlet_int(x->m_out[0], n);
				
				match = 1;
			}
		}
		
		if (!match) {
			outlet_int(x->m_out[1], n);
		}
	}
}

////////////////////////////

void multisplit_float(t_multisplit *x, double f)
{
	
	long i = 0;
	long match = 0;
	
	long inlet = proxy_getinlet((t_object *)x);
	
	
	if (inlet == 0 && x->m_argLen >= 2) {
		
		
		for (i = x->m_argLen-2; i >= 0; i--) {	
			
			
			if ( ((x->m_argv + i + 1)->a_type == A_LONG) && ((x->m_argv + i)->a_type == A_LONG) ){
				
				if ( floatBetween(atom_getlong(x->m_argv + i + 1), atom_getlong(x->m_argv + i), f) == 1 ){
					
					outlet_float(x->m_out[i], f);
					
					match = 1;
				}
			}
			
			
			if ( ((x->m_argv + i + 1)->a_type == A_FLOAT) && ((x->m_argv + i)->a_type == A_LONG) ){
				
				if ( floatBetween(atom_getfloat(x->m_argv + i + 1), atom_getlong(x->m_argv + i), f) ){
					
					outlet_float(x->m_out[i], f);
					
					match = 1;
				}
			}
			
			
			if ( ((x->m_argv + i + 1)->a_type == A_LONG) && ((x->m_argv + i)->a_type == A_FLOAT) ){
				
				if ( floatBetween(atom_getlong(x->m_argv + i + 1), atom_getfloat(x->m_argv + i), f) ){
					
					outlet_float(x->m_out[i], f);
					
					match = 1;
				}
			}
			
			
			if ( ((x->m_argv + i + 1)->a_type == A_FLOAT) && ((x->m_argv + i)->a_type == A_FLOAT) ){
				
				if ( floatBetween(atom_getfloat(x->m_argv + i + 1), atom_getfloat(x->m_argv + i), f) ){
					
					outlet_float(x->m_out[i], f);
					
					match = 1;
				}
			}
		}
		
		if (!match) {
			outlet_float(x->m_out[x->m_argLen - 1], f);
		}
	}
	
	// si l'objet a moins de 2 args :
	
	if (inlet == 0 && x->m_argLen < 2) {
		
		if ( (x->m_argv + 1)->a_type == A_LONG){
			
			if ( floatBetween(0, atom_getlong(x->m_argv + 1), f) ){
				
				outlet_float(x->m_out[0], f);
				
				match = 1;
			}
		}
		if ( (x->m_argv + 1)->a_type == A_FLOAT){
			
			if ( floatBetween(0, atom_getfloat(x->m_argv + 1), f) ){
				
				outlet_float(x->m_out[0], f);
				
				match = 1;
			}
		}
		
		if (!match) {
			outlet_float(x->m_out[1], f);
		}
	}

}

////////////////////////////


void multisplit_list(t_multisplit *x, t_symbol *s, long ac, t_atom *av)
{	

	long i = 0;
	long inlet = proxy_getinlet((t_object *)x);
	
	if (inlet == 0) {
		
		for (i = 0; i < ac; i++){
			if ((av + i)->a_type == A_LONG) {
				multisplit_int(x, atom_getlong(av + i));
			}
			else if ((av + i)->a_type == A_FLOAT) {
				multisplit_float(x, atom_getfloat(av + i));
			}
		}
	}
	
}


/////////////////////////////////////////////////////////////////////////////////////////

void *multisplit_new(t_symbol *s, long argc, t_atom *argv)
{
	t_multisplit *x = NULL;

	x = (t_multisplit *)object_alloc(multisplit_class);
	
	//post("%ld arguments",argc);
	
	long i;
	
	x->m_numOut = argc-1;
	x->m_argLen = argc;
	

	switch (argc) {	
		case 0:
			x->m_out[1] = outlet_new(x, NULL);
			x->m_out[0] = outlet_new(x, NULL);
			
			x->m_numOut = 1;
			atom_setlong(&x->m_argv[0], 0);
			atom_setfloat(&x->m_argv[1], 1.);
			x->m_argLen = 2;
			break;

		case 1:
			if ((argv)->a_type == A_LONG || (argv)->a_type == A_FLOAT) { // une erreur se produit quand on change les arguments !! a resoudre ...
				x->m_out[1] = outlet_new(x, NULL);
				x->m_out[0] = outlet_new(x, NULL);
				
				x->m_numOut = 1;
				
				atom_setlong(&x->m_argv[0], 0);
				
				if ((argv)->a_type == A_LONG) {
					atom_setlong(&x->m_argv[1], atom_getlong(argv));
					x->m_argLen = 2;
				}
				else if ((argv)->a_type == A_FLOAT) {
					atom_setfloat(&x->m_argv[1], atom_getfloat(argv));
					x->m_argLen = 2;
				}
			}
			else {
				object_error((t_object *)x, "args must be int or float"); 
				return (0);
			}
			break;
			
		default:
			for (i = argc-1; i >= 0; i--){
				//x->m_proxy[i + 1] = proxy_new(x, i + 1, &x->m_numIn); // ne regle pas le probleme freeobject.. a regler !!
				if ((argv + i)->a_type == A_LONG || (argv + i)->a_type == A_FLOAT) {
					x->m_out[i] = outlet_new(x, NULL);									// Nbr d'outlet.
					
				}
				else {
					object_error((t_object *)x, "args must be int or float");
					return (0);
				}
			}
			
			for (i = 0; i < argc; i++){
				if ((argv + i)->a_type == A_LONG) {
					atom_setlong(&x->m_argv[i], atom_getlong(argv + i));
					//post("enum args %ld = %ld", i, atom_getlong(x->m_argv + i));
				}
				else if ((argv + i)->a_type == A_FLOAT) {
					atom_setfloat(&x->m_argv[i], atom_getfloat(argv + i));
					//post("enum args %ld = %2f", i, atom_getfloat(x->m_argv + i));
				}
			}
			break;
	}

	return (x);
}
