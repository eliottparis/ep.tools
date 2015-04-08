
// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

/*******************************************************************************************************************
*																												   *
 *								PARIS ELIOTT premier external Max : (05/11)										  *
  *									- - - - - - - - - - - - - - - - -											 *
 *				catch attend de rattraper la valeur stockee avant de coller a la valeur entrante.				  *
*																												   *
********************************************************************************************************************
 
 formule implementee :
 
 $f3 = 1.27  tolerance  /  $f1 = current  /  $f2 = stored value :
 
 if ( $f1 <= $f2 && $f1 >= ($f2 - $f3 ) ) || ( $f1 >= $f2 && $f1 <= ( $f2 + $f3 ) ) then 1 else 0

            *******************************************************************************************/

#include "ext.h"
#include "ext_obex.h"

typedef struct _catch {	// definition de variables pour toutes les instances de l'objet dans le patch.
	t_object p_ob;			// object header.
	long a_flag;			// int : "toggle active" on/off.
	long a_match;			// int : 1 si match / 0 si unmatch.
	long a_lockFunc;		// int : bloque l'envoi vers la fonction test d'outifcatch si 1.
	double a_tolerance;		// float : tolerance ((de 0 a 1)*(maximum/100)).
	double a_store;			// float : valeur stockee quand flag = 0.
	double a_current;		// float : valeur courante envoyée a catch.
	void *a_outlet;			// pointeur => outlet0.
	void *a_outlet1;		// pointeur => outlet1.
	void *a_outlet2;		// pointeur => outlet1.
} t_catch;


// prototypes pour les methodes definies plus bas :
void catch_bang(t_catch *x);					// bang recu dans inlet gauche.
void catch_int(t_catch *x, long n);				// int recu dans inlet gauche.
void catch_float(t_catch *x, double f);			// float recu dans inlet gauche.
void catch_in1(t_catch *x, long n);				// int/float recu dans inlet droit.
void catch_outifCatch(t_catch *x, double f);	// determine si la valeur courante est egale a la valeur store
void catch_setTolerance(t_catch *x, double tol);// modifie la valeur de tolerance.
void catch_setStore(t_catch *x, double sto);	// modifie la valeur stockee.

void catch_assist(t_catch *x, void *b, long m, long a, char *s);
void *catch_new(double tol, double sto);


t_class *catch_class;		// global pointer to the object class - so max can reference the object 


//--------------------------------------------------------------------------

int C74_EXPORT main(void)
{
	t_class *c;
	
	c = class_new("ep.catch", (method)catch_new, (method)NULL, sizeof(t_catch), 0L, A_DEFFLOAT, A_DEFFLOAT, 0);

	class_addmethod(c, (method)catch_bang,		"bang",		0);			 // bang dans le 1er inlet 
	class_addmethod(c, (method)catch_int,		"int",		A_LONG, 0);	 // int dans le 1er inlet (inlet 0).
	class_addmethod(c, (method)catch_float,		"float",	A_FLOAT, 0); // methode pour un float dans le 1er inlet (inlet 0).
    class_addmethod(c, (method)catch_in1,		"in1",		A_LONG, 0);	 // int dans le second inlet (inlet 1).
	class_addmethod(c, (method)catch_setTolerance,		"tolerance",		A_FLOAT, 0);	 // tolerance $f1 mess in inlet gauche.
	class_addmethod(c, (method)catch_setStore,		"set",		A_FLOAT, 0);	 // store $f1 mess in inlet gauche.

    class_addmethod(c, (method)catch_assist,	"assist",	A_CANT, 0);	 // (optional) assistance methode
	
	class_register(CLASS_BOX, c);
	catch_class = c;

	post("catch object by Eliott Paris, v. Beta 05/11",0);	// message au chargement de la classe.
	return 0;
}


//--------------------------------------------------------------------------  Catch New Function :

void *catch_new(double tol, double sto) // tol = 1er float (A_DEFFLOAT) tape comme arg de l'objet, sto = 2eme. (0 si pas tape).
{
	t_catch *x;				// local variable (pointer to a t_catch data structure)

	x = (t_catch *)object_alloc(catch_class); // creation d'une nouvelle instance de l'objet
	
	intin(x,1);					// creation d'un second inlet integer

	x->a_outlet2 = intout(x);	// creation float outlet droit (1) => uncatch value (Through).
	x->a_outlet1 = floatout(x);	// creation float outlet droit (1) => uncatch value (Through).
	x->a_outlet = floatout(x);	// creation float outlet gauche (0) => catch value if active
	
	x->a_current	= 0.;		// valeur courante = 0 par default.
	x->a_store	= sto;			// valeur store = definie par 2nd arg (0 si undefined).
	x->a_flag	= 0;			// toggle on/off = 0 (off) par default.
	x->a_lockFunc	= 0;
	x->a_match	= 1;
	x->a_tolerance = tol;		// valeur tol = definie par 1er arg (0 si undefined).
	
	//post(" tol = %f", tol);
	//post(" sto = %f", sto);
	//post(" nouvelle instance de catch cree dans ce patch.",0); // message a la creation d'une nouvelle instance de l'objet.
	
	return(x);					// retourne la reference de l'instance de l'objet.
}


//--------------------------------------------------------------------------

void catch_assist(t_catch *x, void *b, long m, long a, char *s) // 4 final arguments are always the same for the assistance method
{
	if (m == ASSIST_OUTLET) //outlets
		switch (a) {
		case 0:
			if (x->a_flag == 0) {
				sprintf(s,"catch value (closed)");
			}
			else {
				sprintf(s,"catch value (open)");
			}
			break;
		case 1:
			sprintf(s,"just passing through right inlet value");
			break;
		case 2:
			sprintf(s,"output 1 if catch, 0 if uncatch");
			break;
		}
	else {					//inlets
		switch (a) {	
		case 0:
			sprintf(s,"value in (float)");
			break;
		case 1:
			sprintf(s,"toggle on/off");
			break;
		}
	}
}
//-------------------------------------------------------------------------- Fonctions in :

void catch_setTolerance(t_catch *x, double tol)
{
	if (tol >= 0) {
		x->a_tolerance = tol;
	}
	else {
		x->a_tolerance = tol * -1;	// si tol est negative on la convertit en positive.
	}
}

void catch_setStore(t_catch *x, double sto)
{
	x->a_store = sto;
	x->a_lockFunc = 0;
	//catch_float(x, x->a_current)
}

//--------------------------------------------

void catch_int(t_catch *x, long n)		// int recu dans inlet gauche.
{
	outlet_float(x->a_outlet1, n);			// outlet1 recoit toujours int et float courant.
	x->a_current = n;						// => valeur courante.
	
	if (x->a_flag == 1) {
		catch_outifCatch(x, n);				// appel de la fonction catch_outifcatch
	}
	else if (x->a_store == x->a_current) {		//
		x->a_match = 1;							//
		}										//
		else {									// match ?
		x->a_match = 0;							//
		}
	if (x->a_flag == 1 && x->a_match == 0) {
		outlet_int(x->a_outlet2, 0);			// outlet2 => unCatch (0).
	}
}


void catch_float(t_catch *x, double f)	// float recu dans inlet gauche
{
	outlet_float(x->a_outlet1, f);			// outlet1 recoit toujours int et float courant.
	x->a_current = f;						//  => valeur courante.
	
	if (x->a_flag == 1) {
		catch_outifCatch(x, f);				// appel de la fonction catch_outifcatch
	}
	else if (x->a_store == x->a_current) {		//
			x->a_match = 1;						//
		}										//
		else {									// match ?
			x->a_match = 0;						//
		}
	if (x->a_flag == 1 && x->a_match == 0) {
		outlet_int(x->a_outlet2, 0);			// outlet2 => unCatch (0).
	}
}


void catch_in1(t_catch *x, long n)		// int Toggle On/Off 
{	
	if (n == 0) {
		x->a_flag = 0;
		outlet_int(x->a_outlet2, 0);		// outlet2 => unCatch (0).
		
		if (x->a_match == 1) {				// si store et current etaient egaux on stocke current dans store
			x->a_store = x->a_current;
		}
	}
	else {
		x->a_flag = 1;
		
		if (x->a_store != x->a_current) {	// si store et current sont differents :
			x->a_lockFunc = 0;				// on debloque la fonction test de outifCatch.
		}
		else {
			x->a_lockFunc = 1;
		}

	}
}

void catch_bang(t_catch *x)					// bang recu dans l'inlet gauche. (sert a debugger)
{
	outlet_float(x->a_outlet, x->a_store);	// sort la valeur store dans l'inlet gauche.
	/*
	post("************************");	
	post(" match = %ld", x->a_match);
	post(" flag = %ld", x->a_flag);
	post(" current = %f", x->a_current);
	post(" store = %f", x->a_store);
	post(" tolerance = %f", x->a_tolerance);
	post(" lockFunc = %ld", x->a_lockFunc);
	*/
}

//-------------------------------------------------------------------------- Fonction process :

void catch_outifCatch(t_catch *x, double f)
{	
	if (x->a_store == x->a_current) {		//
		x->a_match = 1;						//
	}										//
	else {									// match ?
		x->a_match = 0;						//
	}
	
	if (x->a_match == 0 && x->a_lockFunc == 0) {
		if ( ((x->a_current <= x->a_store) && (x->a_current >= (x->a_store - x->a_tolerance)))	// si current colle a store
			|| ((x->a_current >= x->a_store) && (x->a_current <= (x->a_store + x->a_tolerance))) ) {
			
			outlet_int(x->a_outlet2, 1);		// outlet2 => Catch (1).
			outlet_float(x->a_outlet, f);		// outlet gauche sort la valeur courante une fois attrapee.
			x->a_store = x->a_current;
			x->a_match	= 1;					// attrapee.
			x->a_lockFunc = 1;					// condition verouillee pour eviter de perdre la valeur attrapee.
		}
		else {
			x->a_match = 0;						// current n'a pas attrape store.
		}
	}
	else {
		outlet_int(x->a_outlet2, 1);			// outlet2 => Catch (1).
		outlet_float(x->a_outlet, f);			// outlet gauche colle a la valeur courante.
		x->a_store = x->a_current;
		x->a_match	= 1;						// attrapee.
		x->a_lockFunc = 1;
	}
}

