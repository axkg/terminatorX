/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999  Alexander K÷nig
 
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

#ifndef WIN32
#include <X11/extensions/XInput.h>
#include <X11/X.h>
#endif

#include "license.c"
#include "tX_wavfunc.h"
#include "tX_mastergui.h"
#include "version.h"

extern char *logo_xpm[];

GdkWindow *opt_window=NULL;
GtkWidget *opt_dialog;
GtkWidget *menu=NULL;

GtkWidget *prefix;
GtkWidget *reset_filectr;
GtkWidget *audio_device;
GtkWidget *use_stdout;
GtkWidget *prelis;

GtkAdjustment *buff_no=NULL;
GtkWidget *buff_no_slider;
GtkAdjustment *buff_size=NULL;
GtkWidget *buff_size_slider;

GtkAdjustment *rec_size;
GtkWidget *rec_size_slider;

GtkAdjustment *sense_cycles=NULL;
GtkWidget *sense_cycles_slider;

GtkAdjustment *vtt_default_speed=NULL;
GtkWidget *vtt_default_speed_slider;

GtkWidget *xinput_enable;
GtkWidget *xinput_device;
GtkAdjustment *mouse_speed=NULL;
GtkWidget *mouse_speed_slider;
GtkWidget *use_y;

GtkWidget *tooltips;
GtkWidget *show_nag;
GtkAdjustment *update_idle=NULL;
GtkWidget *update_idle_slider;

GtkWidget *time_enable;
GtkAdjustment *time_update=NULL;
GtkWidget *time_update_slider;

GtkWidget *opt_ok;
GtkWidget *opt_apply;
GtkWidget *opt_cancel;

GtkTooltips *opt_tips;

int opt_hidden=0;

void apply_options()
{
	char *text;
	
	strcpy(globals.prefix, gtk_entry_get_text(GTK_ENTRY(prefix)));
	globals.reset_filectr=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(reset_filectr));
	
	strcpy(globals.audio_device, gtk_entry_get_text(GTK_ENTRY(audio_device)));
	globals.use_stdout=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(use_stdout));
	globals.buff_no=(int)buff_no->value;	
	globals.buff_size=(int)buff_size->value;
	
	globals.prelis=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(prelis));
	
	globals.rec_size=(int)rec_size->value*1024;
	if (malloc_recbuffer()) tx_note("Error: Failed to allocate recbuffer.");
	
	globals.sense_cycles=(int) sense_cycles->value;
	globals.vtt_default_speed=vtt_default_speed->value;
	globals.xinput_enable=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(xinput_enable));
	gtk_label_get(GTK_LABEL(GTK_BUTTON(xinput_device)->child), &text);
	strcpy(globals.xinput_device, text);	
	globals.use_y=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(use_y));
	
	globals.mouse_speed=mouse_speed->value;
	globals.tooltips=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tooltips));
	globals.show_nag=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(show_nag));
	globals.update_idle=(int) update_idle->value;
	
	globals.time_enable=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(time_enable));
	globals.time_update=(int)time_update->value;
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

void select_input(GtkWidget *w, char *dev)
{
	gtk_label_set(GTK_LABEL(GTK_BUTTON(xinput_device)->child), dev);
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
	Display *dpy;
	
	int i, devmax;
		
	opt_dialog=gtk_dialog_new();
	w=&(GTK_DIALOG(opt_dialog)->window);
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

	add_expl("Turntable Default Speed:");
	
	vtt_default_speed=(GtkAdjustment*) gtk_adjustment_new(globals.vtt_default_speed, -2.5, 2.5, 0.1, 0.01, 0.01);
	vtt_default_speed_slider=gtk_hscale_new(vtt_default_speed);
	gtk_scale_set_digits(GTK_SCALE(vtt_default_speed_slider), 2);
	gtk_scale_set_value_pos(GTK_SCALE(vtt_default_speed_slider), GTK_POS_LEFT);
	gtk_tooltips_set_tip(opt_tips, vtt_default_speed_slider, "Sets the \"motor\" speed of the turntable. 1.0 => real speed, 2.0 => double speed, negative values => play backwards.", NULL);
	add_widget_dyn(vtt_default_speed_slider);
	
	end_box();

	begin_box();

	prelis=gtk_check_button_new_with_label("Pre-Listen to audio files in scratch/loop dialog");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(prelis), globals.prelis);
	add_widget_fix(prelis);		
	
	end_box();

	my_new_subsec("[ Recording: ]");
	
	begin_box();

	add_expl("Fast Save Prefix:");
	
	prefix=gtk_entry_new_with_max_length(PATH_MAX);
	gtk_entry_set_text(GTK_ENTRY(prefix), globals.prefix);
	add_widget_dyn(prefix);
	
	end_box();

	begin_box();

	reset_filectr=gtk_check_button_new_with_label("Reset the file counter on startup");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(reset_filectr), globals.reset_filectr);	
	add_widget_fix(reset_filectr);	
	
	end_box();
	
	begin_box();

	add_expl("Record Buffer Size (KB):");

	rec_size=(GtkAdjustment*) gtk_adjustment_new(globals.rec_size/1024, 100, 50000, 1000, 100, 100);

	rec_size_slider=gtk_hscale_new(rec_size);
	gtk_scale_set_digits(GTK_SCALE(rec_size_slider), 0);
	gtk_scale_set_value_pos(GTK_SCALE(rec_size_slider), GTK_POS_LEFT);
	gtk_tooltips_set_tip(opt_tips,	rec_size_slider, "Sets the size of the buffer that stores recorded scratches. If you want to record longer scratches you need to increase this value.", NULL);
	add_widget_dyn(rec_size_slider);
		
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
	gtk_tooltips_set_tip(opt_tips,	xinput_enable, "Enable this if you want to use other input than your default X-Pointer. You have to select your desired device as well.", NULL);
	add_widget_fix(xinput_enable);
	
	if (strlen(globals.xinput_device)>0)	
	{
		xinput_device=gtk_button_new_with_label(globals.xinput_device);
	}
	else
	{
		xinput_device=gtk_button_new_with_label("< NONE >");
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
	
	sense_cycles=(GtkAdjustment*) gtk_adjustment_new(globals.sense_cycles, 1, 50, 5, 1, 1);
	sense_cycles_slider=gtk_hscale_new(sense_cycles);
	gtk_scale_set_digits(GTK_SCALE(sense_cycles_slider), 0);
	gtk_scale_set_value_pos(GTK_SCALE(sense_cycles_slider), GTK_POS_LEFT);
	gtk_tooltips_set_tip(opt_tips, sense_cycles_slider, "If there is no \"motion-event\" for x cycles, where x is the number of cycles you select here, terminatorX assumes mouse motion has stopped. For smaller buffer sizes (=> shorter cycle times) you might have to increase this value", NULL);
	add_widget_dyn(sense_cycles_slider);
	
	end_box();
	
	begin_box();

	use_y=gtk_check_button_new_with_label("Use Y instead of X axis");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_y), globals.use_y);	
	add_widget_dyn(use_y);		
	
	end_box();
	
	my_new_subsec("[ Graphics / GUI: ]");
	
	begin_box();
	
	tooltips=gtk_check_button_new_with_label("Main Window Tooltips");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tooltips), globals.tooltips);	
	add_widget_dyn(tooltips);	
	
	end_box();
	
	begin_box();
	
	add_expl("Pos Update Idle:");
		
	update_idle=(GtkAdjustment*) gtk_adjustment_new(globals.update_idle, 1, 100, 1, 10, 10);
	update_idle_slider=gtk_hscale_new(update_idle);
	gtk_scale_set_digits(GTK_SCALE(update_idle_slider), 0);
	gtk_scale_set_value_pos(GTK_SCALE(update_idle_slider), GTK_POS_LEFT);
	gtk_tooltips_set_tip(opt_tips,	update_idle_slider, "The update thread will idle for the selcted amount of milliseconds. If you want to have a more responsive display update increase this value - if you have performance problems reduce this value.", NULL);
	add_widget_dyn(update_idle_slider);
		
	end_box();

	begin_box();
	
	time_enable=gtk_check_button_new_with_label("Time Update:");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(time_enable), globals.time_enable);	
	add_widget_fix(time_enable);

	time_update=(GtkAdjustment*) gtk_adjustment_new(globals.time_update, 1, 50, 1, 5, 5);
	time_update_slider=gtk_hscale_new(time_update);
	gtk_scale_set_digits(GTK_SCALE(time_update_slider), 0);
	gtk_scale_set_value_pos(GTK_SCALE(time_update_slider), GTK_POS_LEFT);
	gtk_tooltips_set_tip(opt_tips, time_update_slider, "The update thread will update the time display every n-th position display update, where n is the value you select. If you want to have a more responsive display update increase this value - if you have performance problems reduce this value.", NULL);
	add_widget_dyn(time_update_slider);
	end_box();

	begin_box();

	show_nag=gtk_check_button_new_with_label("Display nagbox on startup while loading data");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_nag), globals.show_nag);	
	add_widget_dyn(show_nag);		
	
	end_box();
	
	
	my_new_button(opt_ok, "Ok");
	gtk_signal_connect(GTK_OBJECT(opt_ok), "clicked", (GtkSignalFunc) ok_options, NULL);
	my_new_button(opt_apply, "Apply");
	gtk_signal_connect(GTK_OBJECT(opt_apply), "clicked", (GtkSignalFunc) apply_options, NULL);
	my_new_button(opt_cancel, "Cancel");
	gtk_signal_connect(GTK_OBJECT(opt_cancel), "clicked", (GtkSignalFunc) options_destroy, NULL);

	
	gtk_widget_show(opt_dialog);
	opt_window=opt_dialog->window;
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
	
	int loop;
	
	if (about) 
	{
		gdk_window_raise(about->window);
		return;
	}
	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);	
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
		add_about_wid(pwid);
		
		sep=gtk_hseparator_new();
		add_about_wid(sep);
		
		label=gtk_label_new(
		"\nThis is "PACKAGE" Release "VERSION" - Copyright (C) 1999 by Alexander König" 
		"\n\nSend comments, patches and scratches to: alkoit00@fht-esslingen.de\n"
		"terminatorX-homepage: http://termX.cjb.net\n\nThis binary has been compiled with the following flags: "
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
		" - \nenhanced scheduling: "
#ifdef USE_SCHEDULER
		"ON"
#else
		"OFF"
#endif
		" - keep device open: "
#ifdef KEEP_DEV_OPEN
		"ON"
#else
		"OFF"
#endif
		" - for a "
#ifdef BIG_ENDIAN_MACHINE
		"big"
		
#else
		"little"
#endif
		" endian machine.\n"	
		);
		gtk_misc_set_alignment (GTK_MISC(label), 0.5 ,0.5);
		add_about_wid(label);
		
		sep=gtk_hseparator_new();
		add_about_wid(sep);

		label=gtk_label_new("License (GPL V2):");
		gtk_misc_set_alignment (GTK_MISC(label), 0.5 ,0.5);
		add_about_wid(label);

		hbox=gtk_hbox_new(FALSE, 5);		
		
		text=gtk_text_new(NULL,NULL);
		scroll=gtk_vscrollbar_new(GTK_TEXT(text)->vadj);
		gtk_text_set_editable(GTK_TEXT(text),0);
		gtk_text_set_word_wrap( GTK_TEXT(text), 0);

		if (!GPL_font)
		{
			GPL_font=gdk_font_load ("-misc-fixed-medium-r-*-*-*-120-*-*-*-*-*-*");
		}		
		gtk_text_insert(GTK_TEXT(text), GPL_font, NULL, NULL, license, strlen(license));

		gtk_box_pack_start(GTK_BOX(hbox), text, WID_DYN);
		gtk_widget_show(text);
		
		gtk_box_pack_start(GTK_BOX(hbox), scroll, WID_FIX);
		gtk_widget_show(scroll);
		
		add_about_wid(hbox);

		sep=gtk_hseparator_new();
		add_about_wid(sep);

		btn=gtk_button_new_with_label("Close");
		add_about_wid(btn);

		gtk_container_add(GTK_CONTAINER(window), box);
		gtk_widget_show(box);
		
		gtk_signal_connect(GTK_OBJECT(btn), "clicked", (GtkSignalFunc) destroy_about, NULL);		
		gtk_signal_connect(GTK_OBJECT(window), "delete-event", (GtkSignalFunc) destroy_about, NULL);		
	}
	gtk_widget_show(window);
	
	about=window;
}
