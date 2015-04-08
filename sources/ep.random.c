
// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

/*******************************************************************************************************************
*																												   *
 *								PARIS ELIOTT 2eme external Max : ep.random (05/11)								  *
  *									- - - - - - - - - - - - - - - - - - 										 *
 *										random value between $1 et $2											  *
*																												   *
********************************************************************************************************************/


#include "ext.h"
#include "ext_obex.h"
#include "stdlib.h"

typedef struct _random {	// definition de variables pour toutes les instances de l'objet dans le patch.
	t_object p_ob;			// object header.
	t_atom a_Min;			// float : Min Value.
	t_atom a_Max;			// float : Max Value.
	long   a_listLen;		// int	 : Taille de la liste (inlet 3).
	short  a_outType;		// Bool	 : 0 = int / 1 = float
	void *a_outlet;			// pointeur => outlet0.
} t_random;


// prototypes pour les methodes definies plus bas :
void random_bang(t_random *x);										// bang recu dans inlet gauche.
void random_in1(t_random *x, double Min);							// int/float recu dans 2eme inlet.
void random_in2(t_random *x, double Max);							// int/float recu dans le 3eme inlet.
void random_in3(t_random *x, long Len);								// int recu dans le 4eme inlet.
//void random_randomBetween(t_random *x, double Min, double Max);		// Retourne nombre aleatoire entre les valeurs Min et Max.

void random_assist(t_random *x, void *b, long m, long a, char *s);
void *random_new(t_symbol *s, long argc, t_atom *argv);


t_class *random_class;		// global pointer to the object class - so max can reference the object 


//--------------------------------------------------------------------------

int C74_EXPORT main(void)
{
	t_class *c;
	
	c = class_new("ep.random", (method)random_new, (method)NULL, sizeof(t_random), 0L, A_GIMME, 0);

	class_addmethod(c, (method)random_bang,		"bang",		0);			 // bang dans le 1er inlet	(inlet 0)
    class_addmethod(c, (method)random_in1,		"ft1",		A_FLOAT, 0); // float dans le 2eme inlet (inlet 1).
	class_addmethod(c, (method)random_in2,		"ft2",		A_FLOAT, 0); // float dans le 3eme inlet (inlet 2).
	class_addmethod(c, (method)random_in3,		"in3",		A_LONG, 0);  // int dans le 4eme inlet (inlet 3).

    class_addmethod(c, (method)random_assist,	"assist",	A_CANT, 0);	 // (optional) assistance methode
	
	class_register(CLASS_BOX, c);
	random_class = c;

	post("ep.random object by Eliott Paris, v. Beta 05/11",0);	// message au chargement de la classe.
	return 0;
}


//--------------------------------------------------------------------------  random New Function :

void *random_new(t_symbol *s, long argc, t_atom *argv) // tol = 1er float (A_DEFFLOAT) tape comme arg de l'objet, sto = 2eme. (0 si pas tape).
{
	t_random *x;				// local variable (pointer to a t_random data structure)

	x = (t_random *)object_alloc(random_class); // creation d'une nouvelle instance de l'objet
	
	intin(x, 3);						// creation d'un quatrieme inlet entier.
	floatin(x,2);						// creation d'un troisieme inlet float.
	floatin(x,1);						// creation d'un second inlet float.

	x->a_outlet = outlet_new(x, NULL);	// creation float outlet gauche (0)
	
	

	//object_post((t_object *)x, "it has %ld arguments", argc);
	
	
	switch (argc) {											// si aucun argument n'est tape, => valeurs par default.
			
		case 0:
			atom_setlong(&x->a_Min, 0);
			atom_setfloat(&x->a_Max, 1);
			x->a_listLen = 1;
			break;
			
		case 1:												// si seulement le 1er argument est tape => affecte val Max, autres valeurs par default.
			atom_setlong(&x->a_Min, 0);
			x->a_listLen = 1;
			
			///// Max
			if ((argv + 0)->a_type == A_LONG) {
				atom_setlong(&x->a_Max, atom_getlong(argv+0));
			}
			else if ((argv + 0)->a_type == A_FLOAT) {
				atom_setfloat(&x->a_Max, atom_getfloat(argv+0));
			}
			else {
				atom_setfloat(&x->a_Max, 1);
				object_error((t_object *)x, "arg 1 must be an int or a float");
			}
			break;
			
		case 2:
			x->a_listLen = 1;
			
			///// Min
			if ((argv + 0)->a_type == A_LONG) {
				atom_setlong(&x->a_Min, atom_getlong(argv+0));
			}
			else if ((argv + 0)->a_type == A_FLOAT) {
				atom_setfloat(&x->a_Min, atom_getfloat(argv+0));
			}
			else {
				atom_setfloat(&x->a_Min, 0);
				object_error((t_object *)x, "arg 1 must be an int or a float");
			}
			
			///// Max
			if ((argv + 1)->a_type == A_LONG) {
				atom_setlong(&x->a_Max, atom_getlong(argv+1));
			}
			else if ((argv + 1)->a_type == A_FLOAT) {
				atom_setfloat(&x->a_Max, atom_getfloat(argv+1));
			}
			else {
				atom_setfloat(&x->a_Max, 1);
				object_error((t_object *)x, "arg 2 must be an int or a float");
			}
			break;
			
		case 3:
			///// Min
			if ((argv + 0)->a_type == A_LONG) {
				atom_setlong(&x->a_Min, atom_getlong(argv+0));
			}
			else if ((argv + 0)->a_type == A_FLOAT) {
				atom_setfloat(&x->a_Min, atom_getfloat(argv+0));
			}
			else {
				atom_setfloat(&x->a_Min, 0);
				object_error((t_object *)x, "arg 1 must be an int or a float");
			}
			///// Max
			if ((argv + 1)->a_type == A_LONG) {
				atom_setlong(&x->a_Max, atom_getlong(argv+1));
			}
			else if ((argv + 1)->a_type == A_FLOAT) {
				atom_setfloat(&x->a_Max, atom_getfloat(argv+1));
			}
			else {
				atom_setfloat(&x->a_Max, 1);
				object_error((t_object *)x, "arg 2 must be an int or a float");
			}
			
			///// listLen
			if ((argv + 2)->a_type == A_LONG) {
				if (atom_getlong(argv+2) <= 1) {
					x->a_listLen = 1;
				}
				else {
					x->a_listLen = atom_getlong(argv+2);
				}
			}
			else if ((argv + 2)->a_type == A_FLOAT) {
				if (atom_getfloat(argv+2) <= 1) {
					x->a_listLen = 1;
				}
				else {
					x->a_listLen = round(atom_getfloat(argv+2));
				}
			}
			else {
				x->a_listLen = 1;
				object_error((t_object *)x, "arg 3 must be an int");
			}
			break;

		default:
			atom_setlong(&x->a_Min, 0);
			atom_setfloat(&x->a_Max, 1);
			x->a_listLen = 1;
			object_error((t_object *)x, "accept only 3 args");
			break;
	}
	
	
	if (x->a_Min.a_type == A_LONG && x->a_Max.a_type == A_LONG) {
		x->a_outType = 0;
		//post("Min et max sont entiers");
	}
	else {
		x->a_outType = 1;
        //post("Min et max sont flottants");
	}

	return(x);					// retourne la reference de l'instance de l'objet.
}


//--------------------------------------------------------------------------

void random_assist(t_random *x, void *b, long m, long a, char *s) // 4 final arguments are always the same for the assistance method
{
	if (m == ASSIST_OUTLET) //outlets
		switch (a) {
		case 0:
			if (&x->a_Max > &x->a_Min) {
				if ((x->a_Min.a_type == A_LONG && x->a_Max.a_type == A_LONG)) {
					sprintf(s,"Random list of int between (%ld to %ld)", atom_getlong(&x->a_Min), atom_getlong(&x->a_Max));
				}
				else {
					sprintf(s,"Random list of float between (%.3f to %.3f)", atom_getfloat(&x->a_Min), atom_getfloat(&x->a_Max));
				}
			}
			else {
				if ((x->a_Min.a_type == A_LONG && x->a_Max.a_type == A_LONG)) {
					sprintf(s,"Random list of int between (%ld to %ld)", atom_getlong(&x->a_Max), atom_getlong(&x->a_Min));
				}
				else {
					sprintf(s,"Random list of float between (%.3f to %.3f)", atom_getfloat(&x->a_Max), atom_getfloat(&x->a_Min));
				}
			}
			break;
		}
	else {					//inlets
		switch (a) {
				
		case 0:
			if (&x->a_Max > &x->a_Min) {
				if ((x->a_Min.a_type == A_LONG && x->a_Max.a_type == A_LONG)) {
					sprintf(s,"Bang outputs a random list of int between (%ld to %ld)", atom_getlong(&x->a_Min), atom_getlong(&x->a_Max));
				}
				else {
					sprintf(s,"Bang outputs a random list of float between (%.3f to %.3f)", atom_getfloat(&x->a_Min), atom_getfloat(&x->a_Max));
				}
			}
			else {
				if ((x->a_Min.a_type == A_LONG && x->a_Max.a_type == A_LONG)) {
					sprintf(s,"Bang outputs a random list of int between (%ld to %ld)", atom_getlong(&x->a_Max), atom_getlong(&x->a_Min));
				}
				else {
					sprintf(s,"Bang outputs a random list of float between (%.3f to %.3f)", atom_getfloat(&x->a_Max), atom_getfloat(&x->a_Min));
				}
			}
			break;
				
		case 1:
			if (x->a_Min.a_type == A_LONG) {
				sprintf(s,"Min value (%ld)", atom_getlong(&x->a_Min));
			}
			else {
				sprintf(s,"Min value (%.3f)", atom_getfloat(&x->a_Min));
			}
			//sprintf(s,"Min value (%.3f)", atom_getfloat(&x->a_Min));
			break;
				
		case 2:
			if (x->a_Max.a_type == A_LONG) {
				sprintf(s,"Max value (%ld)", atom_getlong(&x->a_Max));
			}
			else {
				sprintf(s,"Max value (%.3f)", atom_getfloat(&x->a_Max));
			}
			//sprintf(s,"Max value (%.3f)", atom_getfloat(&x->a_Max));
			break;
				
		case 3:
			sprintf(s,"List length (%ld)", x->a_listLen);
			break;
		}
	}
}
//-------------------------------------------------------------------------- Fonctions in :


void random_in1(t_random *x, double Min)		// Set Min value 
{
	if (x->a_outType == 0) {
		atom_setlong(&x->a_Min, round(Min));
	}
	else {
		atom_setfloat(&x->a_Min, Min);
	}
}

void random_in2(t_random *x, double Max)		// Set Max value 
{	
	if (x->a_outType == 0) {
		atom_setlong(&x->a_Max, round(Max));
	}
	else {
		atom_setfloat(&x->a_Max, Max);
	}
}

void random_in3(t_random *x, long Len)			// Set longueur de la liste.
{
	if (Len <= 1) {
		x->a_listLen = 1;
	}
	else {
		x->a_listLen = round(Len);
	}
}

void random_bang(t_random *x)					// bang recu dans l'inlet gauche.
{
	t_atom myList[x->a_listLen];
	double theNumbers[x->a_listLen];
	short i;
	
	double randvalue;
	
	switch (x->a_listLen) {
		case 1:
			
			if (x->a_outType == 0) {					// si la liste est egale à 1, on sort un int/float
				randvalue = rand()%((atom_getlong(&x->a_Max) +1) - atom_getlong(&x->a_Min)) + atom_getlong(&x->a_Min);
				outlet_int(x->a_outlet, randvalue);
			}
			else {
				randvalue = (rand()/(double)RAND_MAX) * (atom_getfloat(&x->a_Max) - atom_getfloat(&x->a_Min)) + atom_getfloat(&x->a_Min);	
				outlet_float(x->a_outlet, randvalue);
			}
			break;
			
		default:												//sinon on sort une liste.
			if (x->a_outType == 0) {							// si les arguments sont des entiers, on sort une liste d'entier.
				for (i = 0; i < x->a_listLen; i++) {
					
					theNumbers[i] = rand()%((atom_getlong(&x->a_Max) +1) - atom_getlong(&x->a_Min)) + atom_getlong(&x->a_Min);
					
					atom_setlong(myList+i,theNumbers[i]);	// defini un tableau d'entier.
				}
			}
			else {												// si un des arguments est un flottant, on sort une liste de flottants.
				for (i = 0; i < x->a_listLen; i++) {
					
					theNumbers[i] = (rand()/(double)RAND_MAX) * (atom_getfloat(&x->a_Max) - atom_getfloat(&x->a_Min)) + atom_getfloat(&x->a_Min);
					
					atom_setfloat(myList+i,theNumbers[i]);	// defini un tableau de flottants
				}
			}
			
			outlet_list(x->a_outlet, 0L, x->a_listLen, myList);
			break;
	}	
}

//-------------------------------------------------------------------------- Fonction process :
/*
void random_randomBetween(t_random *x, double Min, double Max)
{	
	double randvalue;
	
	randvalue = ( rand()/(double)RAND_MAX ) * (Max - Min) + Min; //permet de generer des flottans.	
	outlet_float(x->a_outlet, randvalue);

	//randvalue = rand()%(Max - Min) + Min; 
	//rand() renvoi un nombre entre 0 et RAND_MAX, % renvoi le reste de la division. => entre 2 et 20.
	// "/(double)RAND_MAX ) *" permet de generer des flottans.
}*/

