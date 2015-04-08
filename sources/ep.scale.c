
// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

/*******************************************************************************************************************
*																												   *
 *								PARIS ELIOTT 2eme external Max : ep.scale (05/11)								  *
  *									- - - - - - - - - - - - - - - - -											 *
 *								 ep.scale est un mix entre l'objet scale et zmap								  *
*																												   *
********************************************************************************************************************/

#include "ext.h"
#include "ext_obex.h"

typedef struct _epScale {	// definition de variables pour toutes les instances de l'objet dans le patch.
	t_object p_ob;			// object header.
	long a_Clip;			// int : Clip on/off.
	double a_LastIn;		// float : Last input Value
	double a_Minin;			// float : Min in Value.
	double a_Maxin;			// float : Max in Value.
	double a_Minout;		// float : Min out Value.
	double a_Maxout;		// float : Max out Value.
	void *a_outlet;			// pointeur => outlet0.
} t_epScale;


// prototypes pour les methodes definies plus bas :
void epScale_bang(t_epScale *x);						// bang recu dans inlet gauche.
void epScale_int(t_epScale *x, long LastIn);			// int recu dans inlet gauche.
void epScale_float(t_epScale *x, double LastIn);		// float recu dans inlet gauche.
void epScale_in1(t_epScale *x, double Minin);			// int/float recu dans inlet 2.
void epScale_in2(t_epScale *x, double Maxin);			// int/float recu dans inlet 3.
void epScale_in3(t_epScale *x, double Minout);			// int/float recu dans inlet 4.
void epScale_in4(t_epScale *x, double Maxout);			// int/float recu dans inlet 5.
void epScale_setClipMode(t_epScale *x, long clip);		// modifie la valeur de tolerance.
void epScale_Scale(t_epScale *x, double InputValue);	// determine si la valeur courante est egale a la valeur Left


void epScale_assist(t_epScale *x, void *b, long m, long a, char *s);
void *epScale_new(double Minin, double Maxin, double Minout, double Maxout);


t_class *epScale_class;		// global pointer to the object class - so max can reference the object 


//--------------------------------------------------------------------------

int C74_EXPORT main(void)
{
	t_class *c;
	
	c = class_new("ep.scale", (method)epScale_new, (method)NULL, sizeof(t_epScale), 0L, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);

	class_addmethod(c, (method)epScale_bang,	"bang",		0);				 // bang dans le 1er inlet 
	class_addmethod(c, (method)epScale_int,		"int",		A_LONG, 0);		 // int   (inlet 0) ScaleIn.
	class_addmethod(c, (method)epScale_float,	"float",	A_FLOAT, 0);	 // float (inlet 0) ScaleIn.
    class_addmethod(c, (method)epScale_in1,		"ft1",		A_FLOAT, 0);	 // float (inlet 2) Minin.
	class_addmethod(c, (method)epScale_in2,		"ft2",		A_FLOAT, 0);	 // float (inlet 3) Maxin.
	class_addmethod(c, (method)epScale_in3,		"ft3",		A_FLOAT, 0);	 // float (inlet 4) Minout.
	class_addmethod(c, (method)epScale_in4,		"ft4",		A_FLOAT, 0);	 // float (inlet 5) Maxout.
	class_addmethod(c, (method)epScale_setClipMode,		"clip",		A_LONG, 0);	 // clip Mode 0 ou 1.

    class_addmethod(c, (method)epScale_assist,	"assist",	A_CANT, 0);	 // (optional) assistance methode
	
	class_register(CLASS_BOX, c);
	epScale_class = c;

	post("ep.scale object by Eliott Paris, v. Beta 06/11",0);	// message au chargement de la classe.
	return 0;
}


//--------------------------------------------------------------------------  epScale New Function :

void *epScale_new(double Minin, double Maxin, double Minout, double Maxout)
{
	t_epScale *x;				// local variable (pointer to a t_epScale data structure)

	x = (t_epScale *)object_alloc(epScale_class); // creation d'une nouvelle instance de l'objet
	
	floatin(x,4);					// creation inlet float 5 (Maxout)
	floatin(x,3);					// creation inlet float 4 (Minout)
	floatin(x,2);					// creation inlet float 3 (Maxin)
	floatin(x,1);					// creation inlet float 2 (Minin)

	x->a_outlet = floatout(x);		// creation float outlet gauche (0)
	
	if ((Minin == 0) && (Maxin == 0) && (Minout == 0) && (Maxout == 0)) {			// si 0 args
		//post("case 0 args");
		x->a_Minin	= 0;			// valeur Minimum Input.
		x->a_Maxin	= 127;			// valeur Maximum Input.
		x->a_Minout	= 0;			// valeur Minimum Output.
		x->a_Maxout	= 1;			// valeur Maximum Output.
	}
	else if ((Maxin == 0) && (Minout == 0) && (Maxout == 0) ) {						// si 1 args
		//post("case 1 args");
		x->a_Minin	= 0;			// valeur Minimum Input.
		x->a_Maxin	= Minin;		// valeur Maximum Input.	=> Minin devient Maxin
		x->a_Minout	= 0;			// valeur Minimum Output.
		x->a_Maxout	= 1;			// valeur Maximum Output.
	}
	else if ((Minout == 0) && (Maxout == 0)) {										// si 2 args
		//post("case 2 args");
		x->a_Minin	= Minin;		// valeur Minimum Input.
		x->a_Maxin	= Maxin;		// valeur Maximum Input.	
		x->a_Minout	= 0;			// valeur Minimum Output.
		x->a_Maxout	= 1;			// valeur Maximum Output.
	}
	else if ((Maxout == 0) && (Minout != 0)) {										// si 3 args
		//post("case 3 args");
		x->a_Minin	= Minin;		// valeur Minimum Input.
		x->a_Maxin	= Maxin;		// valeur Maximum Input.	
		x->a_Minout	= 0;			// valeur Minimum Output.
		x->a_Maxout	= Minout;		// valeur Maximum Output.	=> Minout devient Maxout
	}
	else {																			// si 4 args
		//post("case 4 args");
		x->a_Minin	= Minin;		// valeur Minimum Input.
		x->a_Maxin	= Maxin;		// valeur Maximum Input.
		x->a_Minout	= Minout;		// valeur Minimum Output.
		x->a_Maxout	= Maxout;		// valeur Maximum Output.
	}
	
	x->a_Clip = 0;		
	
	return(x);					// retourne la reference de l'instance de l'objet.
}


//--------------------------------------------------------------------------

void epScale_assist(t_epScale *x, void *b, long m, long a, char *s) // 4 final arguments are always the same for the assistance method
{
	if (m == ASSIST_OUTLET) //outlets
		switch (a) {
		case 0:
			sprintf(s,"Scaled output value");
			break;
		}
	else {					//inlets
		switch (a) {	
		case 0:
			sprintf(s,"Number to be scaled");
			break;
		case 1:
			sprintf(s,"Set Low input value");
			break;
		case 2:
			sprintf(s,"Set High input value");
			break;
		case 3:
			sprintf(s,"Set Low output value");
			break;
		case 4:
			sprintf(s,"Set High output value");
			break;
		}
	}
}
//-------------------------------------------------------------------------- Fonctions in :

void epScale_setClipMode(t_epScale *x, long clip)
{
	if (clip == 0) {
		x->a_Clip = 0;
	}
	else {
		x->a_Clip = 1;
	}
}

//--------------------------------------------

void epScale_int(t_epScale *x, long LastIn)			// int recu dans inlet gauche.
{
	x->a_LastIn = LastIn;					// set Lastin value.
	epScale_Scale(x, LastIn);				// appel la fonction Scale.
}


void epScale_float(t_epScale *x, double LastIn)		// float recu dans inlet gauche
{
	x->a_LastIn = LastIn;					// set Lastin value.
	epScale_Scale(x, LastIn);				// appel la fonction Scale.
}

void epScale_bang(t_epScale *x)						// bang recu dans l'inlet gauche.
{
	epScale_Scale(x, x->a_LastIn);			// appel la fonction Scale.
	//post("Minin = %f, Maxout = %f, Minout = %f, Maxout = %f", x->a_Minin, x->a_Maxin, x->a_Minout, x->a_Maxout);
}

//--------------------------------------------------------------------------

void epScale_in1(t_epScale *x, double Minin)		// Set Minin value 
{	
	x->a_Minin = Minin;
}

void epScale_in2(t_epScale *x, double Maxin)		// Set Maxin value 
{	
	x->a_Maxin = Maxin;
}

void epScale_in3(t_epScale *x, double Minout)		// Set Minout value 
{	
	x->a_Minout = Minout;
}

void epScale_in4(t_epScale *x, double Maxout)		// Set Maxout value 
{	
	x->a_Maxout = Maxout;
}

//-------------------------------------------------------------------------- Fonction process :

void epScale_Scale(t_epScale *x, double InputValue)
{
	double a = ( (x->a_Maxout - x->a_Minout) / (x->a_Maxin - x->a_Minin) );
	double b = -1 * ( (a * x->a_Minin) - x->a_Minout );
	double y = (a * InputValue) + b;
	
	if (x->a_Clip == 0) {
		outlet_float(x->a_outlet, y);
	}
	else {
		if ( ((x->a_Minout < x->a_Maxout) && (y >= x->a_Minout && y <= x->a_Maxout))		// Minout < y < Maxout
			|| ((x->a_Minout > x->a_Maxout) && (y <= x->a_Minout && y >= x->a_Maxout)) ) {	// Maxout < y < Minout
			
			outlet_float(x->a_outlet, y);													// y = y
		}
		else if ((x->a_Minout < x->a_Maxout) && (y >= x->a_Maxout)) {		// Ex : y11 0 10 0 10
			outlet_float(x->a_outlet, x->a_Maxout);							// => : 10
		}
		else if ((x->a_Minout < x->a_Maxout) && (y <= x->a_Minout)) {		// Ex : y-1 0 10 0 10
			outlet_float(x->a_outlet, x->a_Minout);							// => : 0
		}
		else if ((x->a_Minout > x->a_Maxout) && (y >= x->a_Minout)) {		// Ex : y11 0 10 10 0
			outlet_float(x->a_outlet, x->a_Minout);							// => : 10
		}
		else if ((x->a_Minout > x->a_Maxout) && (y <= x->a_Maxout)) {		// Ex : y-1 0 10 10 0
			outlet_float(x->a_outlet, x->a_Maxout);							// => : 0
		}		
	}
}

