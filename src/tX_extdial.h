#ifndef _h_tx_extdial_
#define _h_tx_extdial_

#include <gtk/gtk.h>
#include "tX_dial.h"
#include <stdio.h>

class tX_extdial
{
	GtkWidget *mainbox;
	GtkWidget *subbox;
	GtkWidget *dial;
	GtkWidget *label;
	GtkWidget *entry;
	GtkAdjustment *adj;
	int ignore_adj;
	float fval;
	char sval[30];

	private:
	void s2f() { sscanf(sval, "%f", &fval); /*printf("s2f(): s:%s, f%f\n", sval, fval);*/ };
	void f2s() { sprintf(sval, "%3f", fval); sval[4]=0; /* printf("f2s(): s:%s, f%f\n", sval, fval); */ };
	
	public:
	tX_extdial(const char *l, GtkAdjustment *a);
	~tX_extdial();
	GtkWidget *get_widget() { return mainbox; };
	
	static GtkSignalFunc f_entry(GtkWidget *w, tX_extdial *ed);
	static GtkSignalFunc f_adjustment(GtkWidget *w, tX_extdial *ed);
};

#endif 
