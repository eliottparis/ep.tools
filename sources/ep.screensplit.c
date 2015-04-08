
// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

/**
screensplit	
*/

#include "ext.h"							// standard Max include, always required
#include "ext_obex.h"						// required for new style Max object

////////////////////////// object struct
typedef struct _screensplit 
{
	t_object					ob;			// the object itself (must be first)
	long m_numIn, m_numOut;					// nombre d'inlet, et d'outlet.
	t_atom m_argv[128];						// liste des arguments passes a l'objet.
	t_atom m_columnArray[1024];				// liste du split des colonnes.
	t_atom m_rowArray[1024];				// liste du split des rangees.
	double m_largMin;						// largeur Min de l'espace 2D.
	double m_largMax;						// largeur Max de l'espace 2D.
	double m_hautMin;						// hauteur Min de l'espace 2D.
	double m_hautMax;						// hauteur Max de l'espace 2D.
	long m_column;							// Nombre de colonnes.
	long m_row;								// Nombre de rangees.
	t_atom m_lastX[10];
	t_atom m_lastY[10];
	t_atom m_listOut1[3];					// liste outlet 1 (Column, Row, Value).
	t_atom m_listOut2[1024];				// liste outlet 2 (matrice 0 1).
	long m_argLen;							// Nbr d'args passes a l'objet.
	void *m_proxy[8];						// proxy.
	void *m_out[3];
    
} t_screensplit;

///////////////////////// function prototypes

void screensplit_int(t_screensplit *x, long n);
void screensplit_float(t_screensplit *x, double f);
void screensplit_list(t_screensplit *x, t_symbol *s, long argc, t_atom *argv);

void screensplit_setColumnArray(t_screensplit *x);
void screensplit_setRowArray(t_screensplit *x);

int intBetween(long Value1, long Value2, long Current);
int floatBetween(double Value1, double Value2, double Current);

//// standard set
void *screensplit_new(t_symbol *s, long argc, t_atom *argv);
void screensplit_free(t_screensplit *x);
void screensplit_assist(t_screensplit *x, void *b, long m, long a, char *s);

//////////////////////// global class pointer variable
t_class *screensplit_class;


int C74_EXPORT main(void)
{	
	t_class *c;
	
	c = class_new("ep.screensplit", (method)screensplit_new, (method)screensplit_free, (long)sizeof(t_screensplit), 
				  0L /* leave NULL!! */, A_GIMME, 0);
	
	class_addmethod(c, (method)screensplit_int,		"int",		A_LONG,		0);
	class_addmethod(c, (method)screensplit_float,	"float",	A_FLOAT,	0);
	class_addmethod(c, (method)screensplit_list,	"list",		A_GIMME,	0);
		
	/* you CAN'T call this from the patcher */
    class_addmethod(c, (method)screensplit_assist,		"assist",	A_CANT,		0);  
	
	class_register(CLASS_BOX, c);
	screensplit_class = c;

	post("screensplit object by Eliott Paris");
	return 0;
}

void screensplit_assist(t_screensplit *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) { // inlet
		switch (a) {
			case 0:
				sprintf(s, "(Int/Float) X value to test if it's within a screen range");
				break;
			case 1:
				sprintf(s, "(Int/Float) Y value to test if it's within a screen range");
				break;
			case 2:
				sprintf(s, "(Int/Float) Defines Width Min (now : %f)", x->m_largMin);
				break;
			case 3:
				sprintf(s, "(Int/Float) Defines Width Max (now : %f)", x->m_largMax);
				break;
			case 4:
				sprintf(s, "(Int/Float) Defines Height Min (now : %f)", x->m_hautMin);
				break;
			case 5:
				sprintf(s, "(Int/Float) Defines Height Max (now : %f)", x->m_hautMax);
				break;
			case 6:
				sprintf(s, "(Int) Defines Nbr of Column (now : %ld)", x->m_column);
				break;
			case 7:
				sprintf(s, "(Int) Defines Nbr of Raw (now : %ld)", x->m_row);
				break;
			default:
				break;
		}
	} 
	else {	// outlet
		switch (a) {
			case 2:
				sprintf(s, "Output if Out of the Screen"); 
				break;
			case 1:
				sprintf(s, "list of the cells"); 
				break;
			case 0:
				sprintf(s, "list of Column, Row, Value"); 
				break;
			default:
				break;
		}
	}
}

void screensplit_inletinfo(t_screensplit *x, void *b, long index, char *t) // to show cold inlet
{
	if (index > 0) *t = 1;		// inlet 1 froid.
}

void screensplit_free(t_screensplit *x)
{
	object_free(x->m_proxy);
}

/////////////////////////////////////////////////////////////////////////////////////////

void screensplit_setColumnArray(t_screensplit *x)
{
	long i = 0;
	double columnSplitCoef = ((x->m_largMax - x->m_largMin ) / x->m_column);
	post("columnSplitCoef : %f", columnSplitCoef);
	for (i = 0; i <= x->m_column; i++) {
		atom_setfloat(&x->m_columnArray[i], (columnSplitCoef * i) );
		post("columnArray %ld = %f", i, atom_getfloat(x->m_columnArray + i));
		//post("columnSplitCoef : %f", columnSplitCoef * i);
	}
}

void screensplit_setRowArray(t_screensplit *x)
{
	long i = 0;
	double rowSplitCoef = ((x->m_hautMax - x->m_hautMin ) / x->m_row);
	post("rowSplitCoef : %f", rowSplitCoef);
	for (i = 0; i <= x->m_row; i++) {
		atom_setfloat(&x->m_rowArray[i], (rowSplitCoef * i) );
		post("rowArray %ld = %f", i, atom_getfloat(x->m_rowArray + i));
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

void screensplit_int(t_screensplit *x, long n)
{
	long i = 0;
	long match = 0;
	
	long inlet = proxy_getinlet((t_object *)x);
	
	switch (inlet) {
			
		case 0:												// inlet X => on cherche si il entre dans une colonne.
			
			atom_setlong(x->m_lastX, n);
			
			for (i = x->m_column; i >= 0; i--) {
				if ( floatBetween(atom_getfloat(x->m_columnArray + i + 1), atom_getfloat(x->m_columnArray + i), n) == 1 ){
					
					outlet_int(x->m_out[2], n);
					
					match = 1;
					
					atom_setlong(x->m_listOut1 + 2, 1);
					atom_setlong(x->m_listOut1, i);
					
					//outlet_list(x->m_out[0], 0L, 3, x->m_listOut1);
				}
			}
			break;
			
		case 1:												// inlet Y => on cherche si il entre dans une ligne.
			
			atom_setlong(x->m_lastY, n);

			for (i = x->m_row; i >= 0; i--) {
				if ( floatBetween(atom_getfloat(x->m_rowArray + i + 1), atom_getfloat(x->m_rowArray + i), n) == 1 ){
					
					outlet_int(x->m_out[2], n);
					
					match = 1;
					
					atom_setlong(x->m_listOut1 + 2, 1);
					atom_setlong(x->m_listOut1 + 1, i);
					
					//outlet_list(x->m_out[0], 0L, 3, x->m_listOut1);
				}
			}
			break;
			
		case 2:
			x->m_largMin = n;
			screensplit_setColumnArray(x);
			break;
		case 3:
			x->m_largMax = n;
			screensplit_setColumnArray(x);
			break;
		case 4:
			x->m_hautMin = n;
			screensplit_setRowArray(x);
			break;
		case 5:
			x->m_hautMax = n;
			screensplit_setRowArray(x);
			break;
		case 6:
			x->m_column = n;
			screensplit_setColumnArray(x);
			break;
		case 7:
			x->m_row = n;
			screensplit_setRowArray(x);
			break;
		default:
			break;
	}
	
	outlet_list(x->m_out[0], 0L, 3, x->m_listOut1);
	
}

////////////////////////////

void screensplit_float(t_screensplit *x, double f)
{
	
	//long i = 0;
	//long match = 0;
	
	long inlet = proxy_getinlet((t_object *)x);
		
	switch (inlet) {
		case 2:
			x->m_largMin = f;
			screensplit_setColumnArray(x);
			break;
		case 3:
			x->m_largMax = f;
			screensplit_setColumnArray(x);
			break;
		case 4:
			x->m_hautMin = f;
			screensplit_setRowArray(x);
			break;
		case 5:
			x->m_hautMax = f;
			screensplit_setRowArray(x);
			break;
		case 6:
			x->m_column = round(f);
			screensplit_setColumnArray(x);
			break;
		case 7:
			x->m_row = round(f);
			screensplit_setRowArray(x);
			break;
		default:
			break;
	}
	
}

////////////////////////////


void screensplit_list(t_screensplit *x, t_symbol *s, long ac, t_atom *av)
{
	;
/*
	long i = 0;
	long inlet = proxy_getinlet((t_object *)x);
	
	if (inlet == 0) {
		
		for (i = 0; i < ac; i++){
			if ((av + i)->a_type == A_LONG) {
				screensplit_int(x, atom_getlong(av + i));
			}
			else if ((av + i)->a_type == A_FLOAT) {
				screensplit_float(x, atom_getfloat(av + i));
			}
		}
	}
*/	
}


/////////////////////////////////////////////////////////////////////////////////////////

void *screensplit_new(t_symbol *s, long argc, t_atom *argv)
{
	t_screensplit *x = NULL;

	x = (t_screensplit *)object_alloc(screensplit_class);
	
	//post("%ld arguments",argc);

	long i;
	
	for (i = 7; i >= 1; i--) {
		x->m_proxy[i + 1] = proxy_new(x, i, &x->m_numIn);	// creation de 7 inlets (en plus du premier)
	}
	
	x->m_out[2] = outlet_new(x, NULL);									//
	x->m_out[1] = outlet_new(x, NULL);									///// 3 outlets.
	x->m_out[0] = outlet_new(x, NULL);									//
		
	x->m_numOut = argc-1;
	x->m_argLen = argc;
	
	for (i = 0; i < 6; i++) {
		switch (i) {
			case 0:
				if ((argv + i)->a_type == A_LONG) {
					x->m_largMin = atom_getlong(argv + i);
				}
				else if ((argv + i)->a_type == A_FLOAT) {
					x->m_largMin = atom_getfloat(argv + i);
				}
				else {
					x->m_largMin = 0;
				}
				break;
			case 1:
				if ((argv + i)->a_type == A_LONG) {
					x->m_largMax = atom_getlong(argv + i);
				}
				else if ((argv + i)->a_type == A_FLOAT) {
					x->m_largMax = atom_getfloat(argv + i);
				}
				else {
					x->m_largMax = 200;
				}
				break;
			case 2:
				if ((argv + i)->a_type == A_LONG) {
					x->m_hautMin = atom_getlong(argv + i);
				}
				else if ((argv + i)->a_type == A_FLOAT) {
					x->m_hautMin = atom_getfloat(argv + i);
				}
				else {
					x->m_hautMin = 0;
				}
				break;
			case 3:
				if ((argv + i)->a_type == A_LONG) {
					x->m_hautMax = atom_getlong(argv + i);
				}
				else if ((argv + i)->a_type == A_FLOAT) {
					x->m_hautMax = atom_getfloat(argv + i);
				}
				else {
					x->m_hautMax = 200;
				}
				break;
			case 4:
				if ((argv + i)->a_type == A_LONG) {
					x->m_column = atom_getlong(argv + i);
				}
				else if ((argv + i)->a_type == A_FLOAT) {
					x->m_column = atom_getfloat(argv + i);
				}
				else {
					x->m_column = 2;
				}
				break;
			case 5:
				if ((argv + i)->a_type == A_LONG) {
					x->m_row = atom_getlong(argv + i);
				}
				else if ((argv + i)->a_type == A_FLOAT) {
					x->m_row = atom_getfloat(argv + i);
				}
				else {
					x->m_row = 2;
				}
				break;
			default:
				break;
		}
	}
	
	screensplit_setColumnArray(x);
	screensplit_setRowArray(x);
	
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

	return (x);
}
