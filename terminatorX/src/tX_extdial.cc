#include "tX_extdial.h"
#include <string.h>

#define WID_DYN TRUE, TRUE, 0
#define WID_FIX FALSE, FALSE, 0

GtkSignalFunc tX_extdial :: f_entry(GtkWidget *w, tX_extdial *ed)
{

	strcpy(ed->sval, gtk_entry_get_text(GTK_ENTRY(ed->entry)));
	ed->s2f();
	gtk_adjustment_set_value(ed->adj, ed->fval);
	return NULL;
}

GtkSignalFunc tX_extdial :: f_adjustment(GtkWidget *w, tX_extdial *ed)
{
	ed->fval=ed->adj->value;
	ed->f2s();
	gtk_entry_set_text(GTK_ENTRY(ed->entry), ed->sval);
	return NULL;	
}

tX_extdial :: tX_extdial(const char *l, GtkAdjustment *a)
{
	adj=a;
	fval=adj->value;
	f2s();
	label=gtk_label_new(l);
	dial=gtk_tx_dial_new(adj);
	entry=gtk_entry_new_with_max_length(5);
	gtk_entry_set_text(GTK_ENTRY(entry), sval);
	ignore_adj=0;
	
	mainbox=gtk_vbox_new(FALSE, 0);
	subbox=gtk_hbox_new(TRUE, 0);
	gtk_box_pack_start(GTK_BOX(subbox), dial, WID_FIX);
	gtk_box_pack_start(GTK_BOX(subbox), entry, WID_FIX);
	gtk_box_pack_start(GTK_BOX(mainbox), subbox, WID_FIX);
	gtk_box_pack_start(GTK_BOX(mainbox), label, WID_FIX);
//	gtk_misc_set_alignment(GTK_MISC(label), 0.5, 0.9);
	
	gtk_widget_show(label);
	gtk_widget_show(entry);
	gtk_widget_set_usize(entry, 33, entry->requisition.height);
	gtk_widget_show(dial);
	gtk_widget_show(subbox);
	gtk_widget_show(mainbox);
	
	gtk_signal_connect(GTK_OBJECT(entry), "activate", (GtkSignalFunc) tX_extdial::f_entry, (void *) this);
	gtk_signal_connect(GTK_OBJECT(adj), "value_changed", (GtkSignalFunc) tX_extdial::f_adjustment, (void *) this);
}

tX_extdial :: ~tX_extdial()
{
	gtk_object_destroy(GTK_OBJECT(adj));
	gtk_widget_destroy(entry);
	gtk_widget_destroy(label);
	gtk_widget_destroy(dial);
	gtk_widget_destroy(subbox);
	gtk_widget_destroy(mainbox);
}
