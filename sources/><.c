
// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

/*******************************************************************************************************************
 *																												   *
 *								PARIS ELIOTT 2eme external Max : <> (05/11)										  *
 *									- - - - - - - - - - - - - - - - -											 *
 *				>< est semblable à l'objet "==" mais avec une tolerance => (a peu pres egal)					  *
 *																												   *
 ********************************************************************************************************************
 
 formule implementee :
 
 $f3 = 1.27  tolerance  /  $f1 = Left  /  $f2 = Right value :
 
 if ( $f1 <= $f2 && $f1 >= ($f2 - $f3 ) ) || ( $f1 >= $f2 && $f1 <= ( $f2 + $f3 ) ) then 1 else 0
 
 *******************************************************************************************/

#include "ext.h"
#include "ext_obex.h"

typedef struct _close {	// definition de variables pour toutes les instances de l'objet dans le patch.
	t_object p_ob;			// object header.
	double a_tolerance;		// float : range arround Right.
	double a_Left;			// float : Left Value.
	double a_Right;			// float : Right Value.
	void *a_outlet;			// pointeur => outlet0.
} t_close;


// prototypes pour les methodes definies plus bas :
void close_bang(t_close *x);					// bang recu dans inlet gauche.
void close_int(t_close *x, long Left);				// int recu dans inlet gauche.
void close_float(t_close *x, double Left);			// float recu dans inlet gauche.
void close_in1(t_close *x, double Right);				// int/float recu dans inlet droit.
void close_outifclose(t_close *x);	// determine si la valeur courante est egale a la valeur Left
void close_setTolerance(t_close *x, double tol);// modifie la valeur de tolerance.

void close_assist(t_close *x, void *b, long m, long a, char *s);
void *close_new(double Right, double Tol);


t_class *close_class;		// global pointer to the object class - so max can reference the object 


//--------------------------------------------------------------------------

int main(void)
{
	t_class *c;
	
	c = class_new("><", (method)close_new, (method)NULL, sizeof(t_close), 0L, A_DEFFLOAT, A_DEFFLOAT, 0);
	
	class_addmethod(c, (method)close_bang,		"bang",		0);			 // bang dans le 1er inlet 
	class_addmethod(c, (method)close_int,		"int",		A_LONG, 0);	 // int dans le 1er inlet (inlet 0).
	class_addmethod(c, (method)close_float,		"float",	A_FLOAT, 0); // methode pour un float dans le 1er inlet (inlet 0).
    class_addmethod(c, (method)close_in1,		"ft1",		A_FLOAT, 0);	 // float dans le second inlet (inlet 1).
	class_addmethod(c, (method)close_setTolerance,		"tolerance",		A_FLOAT, 0);	 // tolerance $f1 mess in inlet gauche.
	
    class_addmethod(c, (method)close_assist,	"assist",	A_CANT, 0);	 // (optional) assistance methode
	
	class_register(CLASS_BOX, c);
	close_class = c;
	
	post("\"><\" object by Eliott Paris, v. Beta 05/11",0);	// message au chargement de la classe.
	return 0;
}


//--------------------------------------------------------------------------  close New Function :

void *close_new(double Right, double Tol) // tol = 1er float (A_DEFFLOAT) tape comme arg de l'objet, sto = 2eme. (0 si pas tape).
{
	t_close *x;				// local variable (pointer to a t_close data structure)
	
	x = (t_close *)object_alloc(close_class); // creation d'une nouvelle instance de l'objet
	
	floatin(x,1);					// creation d'un second inlet float
	
	x->a_outlet = intout(x);	// creation int outlet gauche (0)
	
	x->a_Left	= 0.;			// valeur Left = 0 par default.
	x->a_Right	= Right;		// valeur Right = definie par 2nd arg (0 si undefined).
	x->a_tolerance = Tol;		// valeur tol = definie par 2eme arg (0 si undefined).
	
	return(x);					// retourne la reference de l'instance de l'objet.
}


//--------------------------------------------------------------------------

void close_assist(t_close *x, void *b, long m, long a, char *s) // 4 final arguments are always the same for the assistance method
{
	if (m == ASSIST_OUTLET) //outlets
		switch (a) {
			case 0:
				sprintf(s,"Result = [Left > (Right - + tolerance) < Left]");
				break;
		}
	else {					//inlets
		switch (a) {	
			case 0:
				sprintf(s,"Set Right operand, Trigger the calculation");
				break;
			case 1:
				sprintf(s,"Set Right operand");
				break;
		}
	}
}
//-------------------------------------------------------------------------- Fonctions in :

void close_setTolerance(t_close *x, double tol)
{
	if (tol >= 0) {
		x->a_tolerance = tol;
	}
	else {
		x->a_tolerance = tol * -1;	// si tol est negative on la convertit en positive.
	}
}

//--------------------------------------------

void close_int(t_close *x, long Left)		// int recu dans inlet gauche.
{
	x->a_Left = Left;						// set Left value.
	
	close_outifclose(x);				// appel de la fonction close_outifclose
}


void close_float(t_close *x, double Left)	// float recu dans inlet gauche
{
	x->a_Left = Left;						//  set Left value.
	
	close_outifclose(x);				// appel de la fonction close_outifclose
}


void close_in1(t_close *x, double Right)		// Set Right value 
{	
	x->a_Right = Right;
}

void close_bang(t_close *x)					// bang recu dans l'inlet gauche. (sert a debugger)
{
	close_outifclose(x);
	/*
	 post("************************");	
	 post(" Right = %f", x->a_Right);
	 post(" Left = %f", x->a_Left);
	 */
}

//-------------------------------------------------------------------------- Fonction process :

void close_outifclose(t_close *x)
{	
	if ( ((x->a_Right <= x->a_Left) && (x->a_Right >= (x->a_Left - x->a_tolerance)))
		|| ((x->a_Right >= x->a_Left) && (x->a_Right <= (x->a_Left + x->a_tolerance))) ) {
		
		outlet_int(x->a_outlet, 1);		// sort 1 si in range
	}
	else {
		outlet_int(x->a_outlet, 0);		// sort 0 si out range
	}
}

