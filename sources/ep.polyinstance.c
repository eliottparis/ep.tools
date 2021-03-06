
// Copyright (c) 2011-2015 Eliott Paris, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

/*

 bang output the number of poly instances
 - add a @auto (0/1) arg to the object for loadbang functionnality
 
*/

#include "ext.h"
#include "ext_common.h"
#include "ext_obex.h"
#include "jpatcher_api.h"
#include "ep.tools.h"

typedef struct _polyinstance 
{
	t_object	a_ob;
	t_object	*a_patcherview;
	t_pt		a_cached;
	char		a_auto;
	void		*a_out[2];
	long		a_polyfound;
	long		a_voices;
	long		a_index;
    
} t_polyinstance;

t_class *polyinstance_class;

int C74_EXPORT main(void)
{	
	t_class *c;
		
	c = class_new("ep.polyinstance", (method)polyinstance_new, (method)NULL, sizeof(t_polyinstance), 0L, A_GIMME, 0);

    class_addmethod(c, (method)polyinstance_bang,		"bang",			0);
    class_addmethod(c, (method)polyinstance_assist,	"assist",		A_CANT, 0);
	
	CLASS_ATTR_CHAR(c, "auto", 0, t_polyinstance, a_auto);
	CLASS_ATTR_STYLE_LABEL(c, "auto", 0, "onoff", "Auto");
	
	class_register(CLASS_BOX, c);
	polyinstance_class = c;

	return 0;
}

void polyinstance_assist(t_polyinstance *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) { //inlet
		sprintf(s, "bang Reports Total Amount of voices and Instance Index");
	} 
	else {	// outlet
		switch (a) {
			case 0:
				sprintf(s, "Instance Index of Patcher");
				break;
			case 1:
				sprintf(s, "Total Amount of voices");
				break;
			default:
				break;
		}
	}
}

void polyinstance_bang(t_polyinstance *x)
{
	if (x->a_auto)
    {
		outlet_int(x->a_out[1], x->a_voices);
		outlet_int(x->a_out[0], x->a_index);
	}
	else
    {
		polyinstance_getpoly(x);
	}
}

void polyinstance_getpoly(t_polyinstance *x)
{
	t_object *jp;
	t_object *jbx;
	t_object *o;
	t_max_err err;
	
	// get the object's parent patcher
	err = object_obex_lookup(x, gensym("#P"), (t_object **)&jp);
	if (err == MAX_ERR_NONE)
    {
		retest :
		// some kind of patcher in a box
		if (jbx = jpatcher_get_box(jp)) {			// si c'est de type boite (pas poly)
			jp = jpatcher_get_parentpatcher(jp);	// on incremente la hierarchie
			if (jp != NULL) {						// jusqu'au jpatcher principal
				goto retest;
			}
		} 
		
		else {										// un truc qui n'est pas une boite
			t_object *p2;
			t_object *target;
			t_object *nextbox;
			method m;
			
			object_method(jp, gensym("getassoc"), &target);
			if (target) {
				if (m = zgetfn(target, gensym("parentpatcher")))
					(*m)(target, &p2);
				if (p2) {
					nextbox = jpatcher_get_firstobject(p2);
					while (nextbox) {
						o = jbox_get_object(nextbox);
						if (o == target) {									// poly~ dettecte !!!
							//post("%s", object_classname(o)->s_name);
							
							x->a_voices = object_attr_getlong(o, gensym("voices"));
							//post("total amount of voices: %ld", x->a_voices);
							
							if(m = zgetfn(o, gensym("getindex")))
								x->a_index = (long)(*m)(o, jp);
							//post("index: %ld", x->a_index);
							
							outlet_int(x->a_out[1], x->a_voices);
							outlet_int(x->a_out[0], x->a_index);
							
							x->a_polyfound = 1;
							
							return;
						}
						nextbox = jbox_get_nextobject(nextbox);
					}
				}
			}
		}
	}
	
	x->a_polyfound = 0;
	x->a_voices = 0;
	x->a_index = 0;
	outlet_int(x->a_out[1], 0);
	outlet_int(x->a_out[0], 0);

}

void *polyinstance_new(t_symbol *s, long argc, t_atom *argv)
{
	t_polyinstance *x = (t_polyinstance *)object_alloc(polyinstance_class);
	    
	if (x)
    {
		attr_args_process(x, argc, argv);		// regarde si des args sont passe a l'objet (on cherche @auto)
		
		//post("Auto : %ld", x->a_auto);
		
		if (x->a_auto) {
			defer_low(x, (method)polyinstance_getpoly, NULL, 0, NULL);
		}
		
		x->a_out[1] = intout(x);
		x->a_out[0] = intout(x);
	}
	
	return (x);
}