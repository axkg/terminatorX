#ifndef _h_tX_panel_
#define _h_tX_panel_

#include <gtk/gtk.h>

class tX_panel
{
	GtkWidget *container;
	GtkWidget *mainbox;
	GtkWidget *pixmap;
	GtkWidget *topbox;
	GtkWidget *clientbox;
	GtkWidget *clientframe;
	GtkWidget *labelbutton;
	GtkWidget *minbutton;
	int client_hidden;
		
	public:
	tX_panel(const char *name, GtkWidget *par);
	~tX_panel();
	
	GtkWidget *get_widget() {return mainbox;};
	GtkWidget *get_labelbutton() {return labelbutton;}
	void add_client_widget(GtkWidget *w);
	int is_hidden() { return client_hidden; }
	void hide(int i) { gtk_toggle_button_set_active((GTK_TOGGLE_BUTTON(minbutton)), i); } 
	
	static void minimize(GtkWidget *w, tX_panel *p);
};

extern void tX_panel_make_label_bold(GtkWidget *widget);
#endif
