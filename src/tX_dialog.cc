/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2002  Alexander König
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
    File: tX_dialog.c
 
    Description: Contains the implementation of the Options and About
    		 Dialogs. (And some really ugly "WE WANT TO 
		 TYPE LESS" macros)
		 
    Changes:
    
    28 Jul 1999: Now display compiletime settings in the about dialog.
*/    

#include "tX_types.h"
#include "tX_global.h"
#include "tX_dialog.h"
#include <gtk/gtk.h>
#include <string.h>
#include <gdk/gdk.h>
#include "tX_icon.h"

#ifndef WIN32
#include <X11/extensions/XInput.h>
#include <X11/X.h>
#endif

#include "license.c"
#include "tX_mastergui.h"
#include "version.h"

extern char *logo_xpm[];

GdkWindow *opt_window=NULL;
GtkWidget *opt_dialog;
GtkWidget *menu=NULL;

GtkWidget *audio_device;
GtkWidget *use_stdout;
GtkWidget *prelis;

GtkAdjustment *buff_no=NULL;
GtkWidget *buff_no_slider;
GtkAdjustment *buff_size=NULL;
GtkWidget *buff_size_slider;

GtkAdjustment *sense_cycles=NULL;
GtkWidget *sense_cycles_slider;

GtkWidget *xinput_enable;
GtkWidget *xinput_device;
GtkAdjustment *mouse_speed=NULL;
GtkWidget *mouse_speed_slider;

GtkWidget *tooltips;
GtkWidget *show_nag;
GtkAdjustment *update_idle=NULL;
GtkAdjustment *update_delay_adj=NULL;
GtkAdjustment *flash_response;
GtkWidget *update_idle_slider;
GtkWidget *update_delay_slider;

GtkWidget *opt_ok;
GtkWidget *opt_apply;
GtkWidget *opt_cancel;

GtkWidget *but_text;
GtkWidget *but_icon;
GtkWidget *but_both;

GtkWidget *sound_editor;

GtkTooltips *opt_tips;

int opt_hidden=0;

void apply_options() {
	char *text;
	
	strcpy(globals.audio_device, gtk_entry_get_text(GTK_ENTRY(audio_device)));
	globals.buff_no=(int)buff_no->value;	
	globals.buff_size=(int)buff_size->value;
	
	globals.prelis=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(prelis));
		
	globals.sense_cycles=(int) sense_cycles->value;
	globals.xinput_enable=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(xinput_enable));
#ifdef 	USE_GTK2
	text=(char *) gtk_button_get_label(GTK_BUTTON(xinput_device));
#else	
	gtk_label_get(GTK_LABEL(GTK_BUTTON(xinput_device)->child), &text);
#endif	
	strcpy(globals.xinput_device, text);	
	
	globals.mouse_speed=mouse_speed->value;
	globals.tooltips=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tooltips));
	if (globals.tooltips) gtk_tooltips_enable(gui_tooltips);
	else gtk_tooltips_disable(gui_tooltips);
	
	globals.show_nag=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(show_nag));
	globals.update_idle=(int) update_idle->value;
	globals.update_delay=(int) update_delay_adj->value;	
	globals.flash_response=flash_response->value;
	
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(but_text))) globals.button_type=BUTTON_TYPE_TEXT;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(but_icon))) globals.button_type=BUTTON_TYPE_ICON;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(but_both))) globals.button_type=BUTTON_TYPE_BOTH;
	strcpy(globals.file_editor, gtk_entry_get_text(GTK_ENTRY(sound_editor)));
}

#define WID_DYN TRUE, TRUE, 0
#define WID_FIX FALSE, FALSE, 0

#define my_new_subsec(s); \
	separator=gtk_hseparator_new(); \
	gtk_box_pack_start(GTK_BOX(vbox), separator, WID_DYN);\
	gtk_widget_show(separator); \
	label=gtk_label_new(s); \
	gtk_misc_set_alignment (GTK_MISC(label), 0 ,0.5); \
	gtk_box_pack_start(GTK_BOX(vbox), label, WID_DYN); \
	gtk_widget_show(label); 

#define my_new_button(btn, s); \
	btn=gtk_button_new_with_label(s); \
	gtk_box_pack_start(GTK_BOX(aa), btn, WID_DYN); \
	gtk_widget_show(btn);
	

#define begin_box(); box=gtk_hbox_new(FALSE, 5);

#define begin_hom_box(); box=gtk_hbox_new(TRUE, 5);

#define end_box(); gtk_box_pack_start(GTK_BOX(vbox), box, WID_DYN); \
	gtk_widget_show(box);

#define add_widget_dyn(wid); gtk_box_pack_start(GTK_BOX(box), wid, WID_DYN);\
	gtk_widget_show(wid);
	
#define add_widget_fix(wid); gtk_box_pack_start(GTK_BOX(box), wid, WID_FIX);\
	gtk_widget_show(wid);

#define add_expl(s); label=gtk_label_new(s); \
	gtk_misc_set_alignment(GTK_MISC(label), 0.5, 0.5);\
	add_widget_fix(label);

#define add_expl_dyn(s); label=gtk_label_new(s); \
	gtk_misc_set_alignment(GTK_MISC(label), 0.5, 0.5);\
	add_widget_dyn(label);
	
static gint showdevmenu(GtkWidget *widget, GdkEvent *event)
{
	if (event->type == GDK_BUTTON_PRESS) {
		GdkEventButton *bevent = (GdkEventButton *) event; 
		gtk_menu_popup (GTK_MENU (widget), NULL, NULL, NULL, NULL,
		bevent->button, bevent->time);
	return TRUE;
	}
	
	return FALSE;	
}

#ifndef WIN32
XDeviceInfo *xdev=NULL;
#endif

void options_destroy(GtkWidget *widget)
{
	/* Destroying everything that is NOT a direct part of
	  the dialog: adjustments, menu and XDeviceList.
	*/

	gdk_window_hide(opt_window);	
	opt_hidden=1;
	
	gtk_object_destroy(GTK_OBJECT(opt_dialog));

#ifndef WIN32
	XFreeDeviceList(xdev);	
#endif	
	opt_window=NULL;
}

void ok_options(GtkWidget *widget)
{
	apply_options();
	options_destroy(widget);
}

void  use_stdout_changed(GtkWidget *widget)
{
	globals.use_stdout=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	globals.use_stdout_cmdline = 0;
}
void select_input(GtkWidget *w, char *dev)
{
#ifdef 	USE_GTK2
	gtk_button_set_label(GTK_BUTTON(xinput_device), dev);
#else		
	gtk_label_set(GTK_LABEL(GTK_BUTTON(xinput_device)->child), dev);
#endif	
}

void create_options()
{
	GtkWidget *box;
	GtkWidget *vbox;
	GtkWidget *aa;
	GtkWidget *label;
	GtkWidget *separator;
	GtkWindow *w;
	GtkWidget *item;
	GSList *button_type_group;
	
	Display *dpy;
	
	int i, devmax;
		
	opt_dialog=gtk_dialog_new();
	w=&(GTK_DIALOG(opt_dialog)->window);
	gtk_window_set_wmclass(GTK_WINDOW(w), "terminatorX", "tX_options");
	gtk_window_set_title(w, "terminatorX - Options");

	opt_tips=gtk_tooltips_new();
	
	vbox=GTK_WIDGET(GTK_DIALOG(opt_dialog)->vbox);
	gtk_box_set_spacing(GTK_BOX(vbox), 5);
	gtk_container_set_border_width(GTK_CONTAINER(w), 5);
//	gtk_box_set_homogeneous(GTK_BOX(vbox), FALSE);
	aa=GTK_WIDGET(GTK_DIALOG(opt_dialog)->action_area);
	gtk_box_set_spacing(GTK_BOX(aa), 5);
//	gtk_box_set_homogeneous(GTK_BOX(aa), FALSE);
	
	label=gtk_label_new("Options:");
	gtk_misc_set_alignment (GTK_MISC(label), 0.5 ,0.5);	
	gtk_box_pack_start(GTK_BOX(vbox), label, WID_DYN);
	gtk_widget_show(label);
	
	my_new_subsec("[ Audio: ]");
	
	begin_box();

	add_expl("Device:");
	
	audio_device=gtk_entry_new_with_max_length(PATH_MAX);
	gtk_entry_set_text(GTK_ENTRY(audio_device), globals.audio_device);
	gtk_tooltips_set_tip(opt_tips, audio_device, "Enter the path to your audio device here. For most systems this should be /dev/dsp.", NULL);
	add_widget_dyn(audio_device);
	
	end_box();

	begin_box();
	use_stdout=gtk_check_button_new_with_label("Use standard output instead of the above device");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_stdout), globals.use_stdout);	
	gtk_signal_connect(GTK_OBJECT(use_stdout), "clicked", (GtkSignalFunc) use_stdout_changed, NULL);
	add_widget_fix(use_stdout);	
	end_box();	
		
	begin_box();
	
	add_expl("No. of Buffers:");
	
	buff_no=(GtkAdjustment*) gtk_adjustment_new(globals.buff_no, 1, 16, 1, 1, 1);
	buff_no_slider=gtk_hscale_new(buff_no);
	gtk_scale_set_digits(GTK_SCALE(buff_no_slider), 0);
	gtk_scale_set_value_pos(GTK_SCALE(buff_no_slider), GTK_POS_LEFT);
	gtk_tooltips_set_tip(opt_tips, buff_no_slider, "Sets the number of kernel level audio buffers. Actually most systems should run just fine with two.", NULL);
	add_widget_dyn(buff_no_slider);

	end_box();

	begin_box();

	add_expl("Size of Buffers:");
	
	buff_size=(GtkAdjustment*) gtk_adjustment_new(globals.buff_size, 1, 16, 1, 1, 1);
	buff_size_slider=gtk_hscale_new(buff_size);
	gtk_scale_set_digits(GTK_SCALE(buff_size_slider), 0);
	gtk_scale_set_value_pos(GTK_SCALE(buff_size_slider), GTK_POS_LEFT);
	gtk_tooltips_set_tip(opt_tips, buff_size_slider, "Sets the size of the kernel level audio buffers. On slower systems you might have to increase this value (if you hear \"clicks\"). Lower values mean lower latency though.", NULL);
	add_widget_dyn(buff_size_slider);
		
	end_box();

	begin_box();

	begin_box();

	prelis=gtk_check_button_new_with_label("Pre-Listen to audio files");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(prelis), globals.prelis);
	add_widget_fix(prelis);		
	
	end_box();
	
	my_new_subsec("[ Mouse / Input: ]");

#ifndef WIN32	
	dpy=XOpenDisplay(NULL);
	xdev=XListInputDevices(dpy, &devmax);
	XCloseDisplay(dpy);

	if (menu) gtk_object_destroy(GTK_OBJECT(menu));
		
	menu = gtk_menu_new();	
	
	for (i=0; i<devmax; i++)
	{
		item = gtk_menu_item_new_with_label(xdev[i].name);
		gtk_menu_append(GTK_MENU(menu), item);
		gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(select_input), xdev[i].name);
		gtk_widget_show(item);
	}

	begin_box();

	xinput_enable=gtk_check_button_new_with_label("XInput Device:");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(xinput_enable), globals.xinput_enable);		
	gtk_tooltips_set_tip(opt_tips,	xinput_enable, "CAREFUL! Enable this *only* if you want to use an input device than your default X-Pointer (yes, your mouse ;). You have to select your desired device as well. Selecting the default mouse pointer will crash terminatorX so if you want to use that keep this option disabled.", NULL);
	add_widget_fix(xinput_enable);
	
	if (strlen(globals.xinput_device)>0)	
	{
		xinput_device=gtk_button_new_with_label(globals.xinput_device);
	}
	else
	{
		xinput_device=gtk_button_new_with_label("");
	}
		
	gtk_signal_connect_object (GTK_OBJECT (xinput_device), "event", GTK_SIGNAL_FUNC (showdevmenu), GTK_OBJECT (menu));
	add_widget_dyn(xinput_device);
		
	end_box();
	
#endif	
	
	begin_box();
	
	add_expl("Mouse Speed:");
		
	mouse_speed=(GtkAdjustment*) gtk_adjustment_new(globals.mouse_speed, -10, 10, 0.5, 0.1, 0.1);
	mouse_speed_slider=gtk_hscale_new(mouse_speed);
	gtk_scale_set_digits(GTK_SCALE(mouse_speed_slider), 1);
	gtk_scale_set_value_pos(GTK_SCALE(mouse_speed_slider), GTK_POS_LEFT);
	gtk_tooltips_set_tip(opt_tips, mouse_speed_slider, "The speed of your mouse in scratch mode. Use negative values to invert motion.", NULL);
	add_widget_dyn(mouse_speed_slider);
	
	end_box();
	
	begin_box();
	
	add_expl("Stop Sense Cycles:");
	
	sense_cycles=(GtkAdjustment*) gtk_adjustment_new(globals.sense_cycles, 1, 150, 5, 1, 1);
	sense_cycles_slider=gtk_hscale_new(sense_cycles);
	gtk_scale_set_digits(GTK_SCALE(sense_cycles_slider), 0);
	gtk_scale_set_value_pos(GTK_SCALE(sense_cycles_slider), GTK_POS_LEFT);
	gtk_tooltips_set_tip(opt_tips, sense_cycles_slider, "If there is no \"motion-event\" for x cycles, where x is the number of cycles you select here, terminatorX assumes mouse motion has stopped. For smaller buffer sizes (=> shorter cycle times) you might have to increase this value", NULL);
	add_widget_dyn(sense_cycles_slider);
	
	end_box();
	
	my_new_subsec("[ Graphics / GUI: ]");
	
	begin_box();
	
	tooltips=gtk_check_button_new_with_label("Main Window Tooltips");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tooltips), globals.tooltips);	
	add_widget_dyn(tooltips);	
	
	end_box();
	
	begin_box();
	
	add_expl("Update Idle:");
		
	update_idle=(GtkAdjustment*) gtk_adjustment_new(globals.update_idle, 1, 100, 1, 10, 10);
	update_idle_slider=gtk_hscale_new(update_idle);
	gtk_scale_set_digits(GTK_SCALE(update_idle_slider), 0);
	gtk_scale_set_value_pos(GTK_SCALE(update_idle_slider), GTK_POS_LEFT);
	gtk_tooltips_set_tip(opt_tips,	update_idle_slider, "The update thread will idle for the selcted amount of milliseconds. If you want to have a more responsive display update increase this value - if you have performance problems reduce this value.", NULL);
	add_widget_dyn(update_idle_slider);
		
	end_box();

	begin_box();
	
	add_expl("Update Delay:");
		
	update_delay_adj=(GtkAdjustment*) gtk_adjustment_new(globals.update_delay, 0, 15, 1, 10, 10);
	update_delay_slider=gtk_hscale_new(update_delay_adj);
	gtk_scale_set_digits(GTK_SCALE(update_delay_slider), 0);
	gtk_scale_set_value_pos(GTK_SCALE(update_delay_slider), GTK_POS_LEFT);
	gtk_tooltips_set_tip(opt_tips,	update_delay_slider, "How often to update the slow widgets.", NULL);
	add_widget_dyn(update_delay_slider);
		
	end_box();

	begin_box();
	
	add_expl("Flash Decay:  ");
	
	flash_response=GTK_ADJUSTMENT(gtk_adjustment_new(globals.flash_response, 0.8, 0.99, 0.01, 0.01, 0.001));
	item=gtk_hscale_new(flash_response);
	gtk_scale_set_digits(GTK_SCALE(item), 2);
	gtk_scale_set_value_pos(GTK_SCALE(item), GTK_POS_LEFT);
//	gtk_tooltips_set_tip(opt_tips,	update_idle_slider, "The update thread will idle for the selcted amount of milliseconds. If you want to have a more responsive display update increase this value - if you have performance problems reduce this value.", NULL);
	add_widget_dyn(item);
	
	end_box();

	begin_box();
	
	add_expl("Buttons as ");
	but_both=item=gtk_radio_button_new_with_label(NULL, "Text+Icon");
	add_widget_fix(item);
	button_type_group=gtk_radio_button_group(GTK_RADIO_BUTTON(item));	
	but_text=item=gtk_radio_button_new_with_label(button_type_group, "Text");
	button_type_group=gtk_radio_button_group(GTK_RADIO_BUTTON(item));	
	add_widget_fix(item);
	but_icon=item=gtk_radio_button_new_with_label(button_type_group, "Icon");
	add_widget_fix(item);
	
	switch (globals.button_type)
	{
		case BUTTON_TYPE_TEXT:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(but_text), 1);
		break;
		case BUTTON_TYPE_ICON:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(but_icon), 1);
		break;
		case BUTTON_TYPE_BOTH:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(but_both), 1);
		break;
		default: fprintf (stderr, "oops: Unknown button type.\n");
	}
	
	end_box();

	begin_box();	

	show_nag=gtk_check_button_new_with_label("Display nagbox on startup while loading data");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_nag), globals.show_nag);	
	add_widget_dyn(show_nag);		
	
	end_box();

	begin_box();
	
	add_expl("Soundfile editor:");
		
	sound_editor=gtk_entry_new_with_max_length(PATH_MAX);
	gtk_entry_set_text(GTK_ENTRY(sound_editor), globals.file_editor);
	gtk_tooltips_set_tip(opt_tips, sound_editor, "Enter your favourite soundfile editor.", NULL);
	add_widget_dyn(sound_editor);	
	
	end_box();	
	my_new_button(opt_ok, "Ok");
	gtk_signal_connect(GTK_OBJECT(opt_ok), "clicked", (GtkSignalFunc) ok_options, NULL);
	my_new_button(opt_apply, "Apply");
	gtk_signal_connect(GTK_OBJECT(opt_apply), "clicked", (GtkSignalFunc) apply_options, NULL);
	my_new_button(opt_cancel, "Cancel");
	gtk_signal_connect(GTK_OBJECT(opt_cancel), "clicked", (GtkSignalFunc) options_destroy, NULL);

	
	gtk_widget_show(opt_dialog);
	opt_window=opt_dialog->window;
	tX_set_icon(opt_dialog, "tX Options");
	gtk_signal_connect(GTK_OBJECT(opt_dialog), "delete-event", (GtkSignalFunc) options_destroy, NULL);	

}

void display_options()
{
        if (opt_window)
	{
		gdk_window_raise(opt_window);	
        }
        else
	{
		create_options();
	}
}

GtkWidget *about=NULL;

void raise_about()
{
	if (about)
	gdk_window_raise(about->window);
}


void destroy_about()
{
	if (about)
	{	
		gtk_widget_destroy(about);
		about=NULL;
	}
}



#define add_about_wid(wid); gtk_box_pack_start(GTK_BOX(box), wid, WID_DYN); \
	gtk_widget_show(wid);

#define add_about_wid_fix(wid); gtk_box_pack_start(GTK_BOX(box), wid, WID_FIX); \
	gtk_widget_show(wid);

GdkFont *GPL_font=NULL;

void show_about(int nag)
{
	GtkWidget *window, *pwid;
	GdkBitmap *mask;
	GtkStyle *style;
	GtkWidget *btn;
	GtkWidget *box;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *sep;
	GtkWidget *text;
	GtkWidget *scroll;
	GdkPixmap *pmap=NULL;
	
	if (about) 
	{
		gdk_window_raise(about->window);
		return;
	}
	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);	
	gtk_window_set_wmclass(GTK_WINDOW(window), "terminatorX", "tX_about");

	gtk_container_set_border_width(GTK_CONTAINER(window), 5);

//	GTK_WINDOW(window)->use_uposition=TRUE;

	gtk_widget_realize(window);
	
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(window), "terminatorX - About");
	
	if (nag)
	{
		gdk_window_set_decorations(window->window, (enum GdkWMDecoration) 0);
	}

	
	style = gtk_widget_get_style( window );

	if (!pmap)
	{
		pmap=gdk_pixmap_create_from_xpm_d(window->window, &mask, &style->bg[GTK_STATE_NORMAL], (gchar **)logo_xpm );
	}


  	pwid = gtk_pixmap_new( pmap, mask );
	
	gtk_widget_show( pwid );

	if (nag)
	{
		gtk_container_add(GTK_CONTAINER(window), pwid);
		gtk_widget_show(window);
		
		while (gtk_events_pending()) gtk_main_iteration();	
	}
	else
	{
		box=gtk_vbox_new(FALSE, 5);
		add_about_wid_fix(pwid);
		
		sep=gtk_hseparator_new();
		add_about_wid_fix(sep);
#ifdef USE_GTK2
		char about_prefix_umlaut[]="\nThis is "PACKAGE" Release "VERSION" - Copyright (C) 1999-2002 by Alexander K\xC3\xB6nig";
#else
		char about_prefix_umlaut[]="\nThis is "PACKAGE" Release "VERSION" - Copyright (C) 1999-2002 by Alexander König";
		char about_prefix_broken_umlaut[]="\nThis is "PACKAGE" Release "VERSION" - Copyright (C) 1999-2002 by Alexander Ko\"nig";
#endif		
		char about_rest[]="\n\nSend comments, patches and scratches to: alex@lisas.de\n"
		"terminatorX-homepage: http://www.terminatorX.cx\n\nThis binary has been compiled with the following flags: "
		"Sox support: "
#ifdef USE_SOX_INPUT
		"ON"
#else
	 	"OFF"
#endif		
		" - mpg123 support: "
#ifdef USE_MPG123_INPUT
		"ON"
#else
		"OFF"
#endif
		" - \nogg123 support: "
#ifdef USE_OGG123_INPUT
		"ON"
#else
		"OFF"
#endif

		" - 3DNow!: "
#ifdef USE_3DNOW
		"ON"
#else
		"OFF"
#endif
		
		" - enhanced scheduling: "
#ifdef USE_SCHEDULER
		"ON"
#else
		"OFF"
#endif
		" - for a "
#ifdef WORDS_BIGENDIAN
		"big"
		
#else
		"little"
#endif
		" endian machine.\n";
		
		char buffer[4096];
		
		strcpy(buffer, about_prefix_umlaut);
		strcat(buffer, about_rest);
		
		label=gtk_label_new(buffer);

#ifndef USE_GTK2
		gtk_label_get(GTK_LABEL(label), &str);
		
		/* Fixing a strange gtk+ bug that appears at least on my system.
		*/
		if (strlen(str)==0) 
		{
			fprintf (stderr, "tX: Warning: this gtk+ has broken umlauts.\n");
			strcpy(buffer, about_prefix_broken_umlaut);
			strcat(buffer, about_rest);
			gtk_label_set(GTK_LABEL(label), buffer);		
		}
#endif
		
		gtk_misc_set_alignment (GTK_MISC(label), 0.5 ,0.5);
		add_about_wid_fix(label);
		
		sep=gtk_hseparator_new();
		add_about_wid_fix(sep);

		label=gtk_label_new("License (GPL V2):");
		gtk_misc_set_alignment (GTK_MISC(label), 0.5 ,0.5);
		add_about_wid_fix(label);

		hbox=gtk_hbox_new(FALSE, 5);		

#ifdef USE_GTK2
		GtkTextIter iter;
		GtkTextBuffer *tbuffer;

		text=gtk_text_view_new();
		tbuffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
		gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text), GTK_WRAP_NONE);
		gtk_text_view_set_editable(GTK_TEXT_VIEW(text), false);
		gtk_text_buffer_get_iter_at_offset (tbuffer, &iter, 0);
		
		scroll=gtk_scrolled_window_new (NULL, NULL);
        	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_container_add (GTK_CONTAINER (scroll), text);
		gtk_text_buffer_create_tag (tbuffer, "courier", "family", "courier", NULL);
		
		gtk_text_buffer_insert_with_tags_by_name(tbuffer, &iter, license, -1, "courier", NULL);
		gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text), 5);
		gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text), 5);
		gtk_widget_set_usize(GTK_WIDGET(text), 640, 180);
		gtk_widget_show(text);		
		
		gtk_box_pack_start(GTK_BOX(hbox), scroll, WID_DYN);
		gtk_widget_show(scroll);		
#else
		text=gtk_text_new(NULL,NULL);
		scroll=gtk_vscrollbar_new(GTK_TEXT(text)->vadj);
		gtk_text_set_editable(GTK_TEXT(text),0);
		gtk_text_set_word_wrap( GTK_TEXT(text), 0);
		gtk_text_insert(GTK_TEXT(text), NULL, NULL, NULL, license, strlen(license));

		gtk_box_pack_start(GTK_BOX(hbox), text, WID_DYN);
		gtk_widget_show(text);
		
		gtk_box_pack_start(GTK_BOX(hbox), scroll, WID_FIX);
		gtk_widget_show(scroll);
#endif		

		
		add_about_wid(hbox);

		sep=gtk_hseparator_new();
		add_about_wid_fix(sep);

		btn=gtk_button_new_with_label("Close");
		add_about_wid_fix(btn);

		gtk_container_add(GTK_CONTAINER(window), box);
		gtk_widget_show(box);
		
		gtk_signal_connect(GTK_OBJECT(btn), "clicked", (GtkSignalFunc) destroy_about, NULL);		
		gtk_signal_connect(GTK_OBJECT(window), "delete-event", (GtkSignalFunc) destroy_about, NULL);		
	}
	gtk_widget_show(window);
	tX_set_icon(window, "tX About");
	
	about=window;
}

GdkBitmap *tX_icon_mask=NULL;
GdkPixmap *tX_icon_pmap=NULL;
GtkWidget *tX_icon_widget=NULL;

void tX_set_icon(GtkWidget *widget, char *name)
{
	GtkStyle *style;

	style = gtk_widget_get_style( widget );

	if (!tX_icon_pmap)
	{
		tX_icon_pmap=gdk_pixmap_create_from_xpm_d(widget->window, &tX_icon_mask, &style->bg[GTK_STATE_NORMAL], (gchar **) tX_icon_xpm );
	  	//tX_icon_widget = gtk_pixmap_new( tX_icon_pmap, tX_icon_mask );		
		//gtk_widget_realize(tX_icon_widget);		
	}

	gdk_window_set_icon(widget->window, NULL, tX_icon_pmap, tX_icon_mask);
	gdk_window_set_icon_name(widget->window, name);	
}
