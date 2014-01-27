/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2014  Alexander König
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
    File: tX_dialog.cc
 
    Description: Contains the implementation of the Options and About
    		 Dialogs. (And some really ugly "WE WANT TO 
		 TYPE LESS" macros)
*/    

#include "config.h"
#include "tX_types.h"
#include "tX_global.h"
#include "tX_dialog.h"
#include <gtk/gtk.h>
#include <string.h>
#include <gdk/gdk.h>
#include "tX_icon.h"
#include "tX_ui_interface.h"
#include "tX_ui_support.h"

#include "tX_widget.h"
#include "tX_flash.h"

#ifndef WIN32
#include <X11/extensions/XInput.h>
#include <X11/X.h>
#endif

#include "license.c"
#include "tX_mastergui.h"
#include "version.h"
#include "tX_vtt.h"
#include <dirent.h>
#include "tX_engine.h"

#ifdef USE_SCHEDULER
#include <sched.h>
#include <pthread.h>
#endif

#ifdef USE_JACK
extern void jack_check();
#endif

extern char *logo_xpm[];
GtkWidget *opt_dialog;
int opt_hidden=0;

static GtkWidget *last_alsa_device_widget=NULL;

void apply_options(GtkWidget *dialog) {
	/* Audio */
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "alsa_driver")))) {
		globals.audiodevice_type=ALSA;
	} else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "oss_driver")))) {
		globals.audiodevice_type=OSS;
	} else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "jack_driver")))) {
		globals.audiodevice_type=JACK;
	}
	globals.use_realtime=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "use_realtime")));
	
	/* Audio: OSS */
	char *oss_device = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(lookup_widget(dialog, "oss_audio_device")));
	if (oss_device) {
		strcpy(globals.oss_device, oss_device);
	}
	globals.oss_buff_no=(int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(lookup_widget(dialog, "oss_buffers")));
	globals.oss_buff_size=(int) gtk_range_get_value(GTK_RANGE(lookup_widget(dialog, "oss_buffersize")));
	globals.oss_samplerate=atoi(gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(lookup_widget(dialog, "oss_samplerate"))));
	
	/* Audio: ALSA */
	char *alsa_device = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(lookup_widget(dialog, "alsa_audio_device")));
	if (alsa_device) {
		strcpy(globals.alsa_device_id, alsa_device);
	}
	globals.alsa_buffer_time=(int) gtk_range_get_value(GTK_RANGE(lookup_widget(dialog, "alsa_buffer_time")));
	globals.alsa_buffer_time*=1000;
	globals.alsa_period_time=(int) gtk_range_get_value(GTK_RANGE(lookup_widget(dialog, "alsa_period_time")));
	globals.alsa_period_time*=1000;
	globals.alsa_samplerate=atoi(gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(lookup_widget(dialog, "alsa_samplerate"))));	
	globals.alsa_free_hwstats=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "alsa_free_hwstats")));
	
	/* TODO: JACK
	*/
	
	/* Input */
	globals.xinput_enable=(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "xinput_enable")))==TRUE);
	char *xinput_device = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(lookup_widget(dialog, "xinput_device")));
	if (xinput_device) {
		strcpy(globals.xinput_device, xinput_device);
	}
	globals.mouse_speed=gtk_range_get_value(GTK_RANGE(lookup_widget(dialog, "mouse_speed")));
	globals.sense_cycles=(int) gtk_range_get_value(GTK_RANGE(lookup_widget(dialog, "stop_sense_cycles")));
	globals.vtt_inertia=gtk_range_get_value(GTK_RANGE(lookup_widget(dialog, "vtt_inertia")));
	
	/* User Interface */ 
	globals.show_nag=(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "startup_nagbox")))==TRUE);
	globals.tooltips=(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "mainwin_tooltips")))==TRUE);
	globals.filename_length=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(dialog, "filename_length")));
//	if (globals.tooltips) gtk_tooltips_enable(gui_tooltips);
//	else gtk_tooltips_disable(gui_tooltips);
//	TODO: Disable Tooltips no longer possible?

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "buttons_text_only")))) {
		globals.button_type=BUTTON_TYPE_TEXT;
	} else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "buttons_icon_only")))) {
		globals.button_type=BUTTON_TYPE_ICON;
	} else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "buttons_text_and_icon")))) {
		globals.button_type=BUTTON_TYPE_BOTH;
	}
	
	globals.update_delay=(int) gtk_range_get_value(GTK_RANGE(lookup_widget(dialog, "update_delay")));
	globals.update_idle=(int) gtk_range_get_value(GTK_RANGE(lookup_widget(dialog, "update_idle")));
	globals.flash_response=gtk_range_get_value(GTK_RANGE(lookup_widget(dialog, "vumeter_decay")));
	
	/* Audio Colors */
	
	strcpy(globals.wav_display_bg_focus, (char *) g_object_get_data(G_OBJECT(lookup_widget(dialog, "wav_display_bg_focus")), "Color"));
	strcpy(globals.wav_display_bg_no_focus, (char *) g_object_get_data(G_OBJECT(lookup_widget(dialog, "wav_display_bg_no_focus")), "Color"));
	strcpy(globals.wav_display_fg_focus, (char *) g_object_get_data(G_OBJECT(lookup_widget(dialog, "wav_display_fg_focus")), "Color"));
	strcpy(globals.wav_display_fg_no_focus, (char *) g_object_get_data(G_OBJECT(lookup_widget(dialog, "wav_display_fg_no_focus")), "Color"));
	strcpy(globals.wav_display_cursor, (char *) g_object_get_data(G_OBJECT(lookup_widget(dialog, "wav_display_cursor")), "Color"));
	strcpy(globals.wav_display_cursor_mute, (char *) g_object_get_data(G_OBJECT(lookup_widget(dialog, "wav_display_cursor_mute")), "Color"));

	/* VU Colors */ 
	strcpy(globals.vu_meter_bg, (char *) g_object_get_data(G_OBJECT(lookup_widget(dialog, "vu_meter_bg")), "Color"));
	strcpy(globals.vu_meter_loud, (char *) g_object_get_data(G_OBJECT(lookup_widget(dialog, "vu_meter_loud")), "Color"));
	strcpy(globals.vu_meter_normal, (char *) g_object_get_data(G_OBJECT(lookup_widget(dialog, "vu_meter_normal")), "Color"));

	globals.vu_meter_border_intensity=gtk_range_get_value(GTK_RANGE(lookup_widget(dialog, "vu_meter_border_intensity")));
	
	/* Misc */
	strcpy(globals.file_editor, gtk_entry_get_text(GTK_ENTRY(lookup_widget(dialog, "soundfile_editor"))));
	strcpy(globals.lrdf_path, gtk_entry_get_text(GTK_ENTRY(lookup_widget(dialog, "ladspa_rdf_path"))));
	globals.compress_set_files=(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "compress_set_files")))==TRUE);	
	globals.prelis=(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "prelisten_enabled")))==TRUE);
	globals.restore_midi_connections=(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "reconnect_enabled")))==TRUE);
	globals.quit_confirm=(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "quit_confirm")))==TRUE);
	globals.verbose_plugin_loading=(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "verbose_plugin_loading")))==TRUE);
	globals.force_nonrt_plugins=(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "force_nonrt_plugins")))==TRUE);
	
	/* update colors */
	std::list<vtt_class *>::iterator vtt;
	
	for (vtt=vtt_class::main_list.begin(); vtt!=vtt_class::main_list.end(); vtt++) {
		gtk_tx_update_colors(GTK_TX((*vtt)->gui.display));
		gtk_widget_queue_draw((*vtt)->gui.display);
		gtk_tx_flash_update_colors(GTK_TX_FLASH((*vtt)->gui.flash));
		gtk_widget_queue_draw((*vtt)->gui.flash);
	}
	
	gtk_tx_flash_update_colors(GTK_TX_FLASH(main_flash));
	gtk_widget_queue_draw(main_flash);
	
#ifdef USE_JACK
	jack_check();
#endif	
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

#ifdef USE_ALSA
void append_alsa_device_list(GtkComboBoxText *combo, char *current) {
	FILE *file;
	char buffer[PATH_MAX+1];
	int ctr = 0;
	
	if ((file = fopen("/proc/asound/pcm", "r"))) {
		while(fgets(buffer, PATH_MAX, file)) {
			buffer[PATH_MAX]=0;
			if (strlen(buffer)) buffer[strlen(buffer)-1]=0;
			if(strstr(buffer, "playback")) {
				char foo[PATH_MAX];
				char tmp[PATH_MAX];
				memset(foo, 0, PATH_MAX);
				int card;
				int device;
 
				sscanf(buffer, "%i-%i: %1024c", &card, &device, foo);
				sprintf(tmp, "hw:%i,%i# %s", card, device, foo);
				
				gtk_combo_box_text_append_text(combo, strdup(tmp));
				
				if (strcmp(tmp, current) == 0) {
					gtk_combo_box_set_active(GTK_COMBO_BOX(combo), ctr);
				}
				ctr++;
					
			}
		}
		fclose(file);
	}
}
#else
void append_alsa_device_list(GtkComboBoxText *combo) {
}
#endif


#ifdef USE_OSS
int oss_select_dsp_only(const struct dirent *entry){
	return (strstr(entry->d_name, "dsp")!=0);
}

void append_oss_device_list(GtkComboBoxText *combo, char *current) {
    struct dirent **namelist;
    int n,i;
    n = scandir("/dev", &namelist, oss_select_dsp_only, alphasort);
    
    if (n>0) {
    	for (i=0; i<n; i++) {
			char buffer[256];
            sprintf(buffer, "/dev/%s", namelist[i]->d_name);
            free(namelist[i]);
            gtk_combo_box_text_append_text(combo, strdup(buffer));
            
            if (strcmp(buffer, current)==0) {
				gtk_combo_box_set_active(GTK_COMBO_BOX(combo), i);
			}
		}
	}
}
#endif

void append_sampling_rates_list(GtkComboBoxText *combo, int current) {

	gtk_combo_box_text_append_text(combo,  "22000");
	gtk_combo_box_text_append_text(combo,  "32000");
	gtk_combo_box_text_append_text(combo,  "44100");
	gtk_combo_box_text_append_text(combo,  "48000");
	switch (current) {
		case 22000:
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
			break;
		case 32000:
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 1);
			break;
		case 44100:
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 2);
			break;
		case 48000:
		default:
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 3);
	}
}

void append_xinput_devices_list(GtkComboBoxText *combo, char *current) {
	int devmax;
	Display *dpy=XOpenDisplay(NULL);
	XDeviceInfo *xdev=XListInputDevices(dpy, &devmax);
	XCloseDisplay(dpy);

	for (int i=0; i<devmax; i++) {		
		gtk_combo_box_text_append_text(combo, strdup(xdev[i].name));
		if (strcmp(xdev[i].name, current) == 0) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo), i);
		}
	}
	
	XFreeDeviceList(xdev);
}

#define MAX_COLORS 10
char *colors[MAX_COLORS]={ NULL };

#define set_color_button(s,g); \
	sprintf(tmp, "<span foreground=\"%s\"><b>%s</b></span>", s, s);\
	gtk_label_set_markup(GTK_LABEL(gtk_container_get_children(GTK_CONTAINER(lookup_widget(dialog, g)))->data), tmp);\
	strcpy(colors[ctr], s);\
	g_object_set_data(G_OBJECT(lookup_widget(dialog, g)), "Color", colors[ctr]);\
	ctr++;


void init_tx_options(GtkWidget *dialog) {
	char tmp[256];
	
	if (colors[0]==NULL) {
		for (int i=0; i<MAX_COLORS; i++) {
			colors[i]=new char[8];
			colors[i][0]=0;
		}
	}
	
	/* Audio */
	switch (globals.audiodevice_type) {		
		case ALSA: gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "alsa_driver")), 1);
			break;
		
		case JACK: gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "jack_driver")), 1);
			break;

		case OSS: 
		default:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "oss_driver")), 1);
			break;
	}

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "use_realtime")), globals.use_realtime);

#ifndef USE_CAPABILITIES
	/* rt's not configurable without capabilities. */
	gtk_widget_hide(lookup_widget(dialog, "use_realtime"));
	gtk_widget_hide(lookup_widget(dialog, "use_realtime_label"));	
#endif
	
#ifndef USE_OSS
	gtk_widget_set_sensitive(lookup_widget(dialog, "oss_driver"), 0);
	gtk_widget_set_sensitive(lookup_widget(dialog, "oss_audio_device"), 0);
	gtk_widget_set_sensitive(lookup_widget(dialog, "oss_buffers"), 0);
	gtk_widget_set_sensitive(lookup_widget(dialog, "oss_buffersize"), 0);
#endif
	
#ifndef USE_ALSA
	gtk_widget_set_sensitive(lookup_widget(dialog, "alsa_driver"), 0);	
	// TODO: Rest!	
#endif
	
#ifndef USE_JACK
	gtk_widget_set_sensitive(lookup_widget(dialog, "jack_driver"), 0);
#endif	
	
	/* Audio: OSS */
	append_oss_device_list(GTK_COMBO_BOX_TEXT(lookup_widget(dialog, "oss_audio_device")), globals.oss_device);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(lookup_widget(dialog, "oss_buffers")), globals.oss_buff_no);
	gtk_range_set_value(GTK_RANGE(lookup_widget(dialog, "oss_buffersize")), globals.oss_buff_size);
	gtk_widget_set_tooltip_text(lookup_widget(dialog, "oss_buffersize"), "Set the size of the kernel level audio buffers. On slower systems you might have to increase this value (if you hear \"clicks\" or drop-outs). Lower values mean lower latency though.");	
	append_sampling_rates_list(GTK_COMBO_BOX_TEXT(lookup_widget(dialog, "oss_samplerate")), globals.oss_samplerate);
	
	/* Audio: ALSA */
	GtkComboBoxText *combo=GTK_COMBO_BOX_TEXT(lookup_widget(dialog, "alsa_audio_device"));
	last_alsa_device_widget=NULL;
	
	append_alsa_device_list(GTK_COMBO_BOX_TEXT(combo), globals.alsa_device_id);

	gtk_range_set_value(GTK_RANGE(lookup_widget(dialog, "alsa_buffer_time")), globals.alsa_buffer_time/1000);
	gtk_widget_set_tooltip_text(lookup_widget(dialog, "alsa_buffer_time"), "Sets the size of the ALSA ring buffer. On slower systems you might have to increase this value (if you hear \"clicks\" or drop-outs). Lower values mean lower latency though.");	
	gtk_range_set_value(GTK_RANGE(lookup_widget(dialog, "alsa_period_time")), globals.alsa_period_time/1000);
	gtk_widget_set_tooltip_text(lookup_widget(dialog, "alsa_period_time"), "The ALSA period time determines how much audio data will be written to the device at once. It is recommended to set this value to a half or a third of the ALSA buffer time.");	

	append_sampling_rates_list(GTK_COMBO_BOX_TEXT(lookup_widget(dialog, "alsa_samplerate")), globals.alsa_samplerate);
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "alsa_free_hwstats")), globals.alsa_free_hwstats);
	
	/* Input */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "xinput_enable")), globals.xinput_enable);
	
	append_xinput_devices_list(GTK_COMBO_BOX_TEXT(lookup_widget(dialog, "xinput_device")), globals.xinput_device);

	gtk_range_set_value(GTK_RANGE(lookup_widget(dialog, "mouse_speed")), globals.mouse_speed);
	gtk_widget_set_tooltip_text(lookup_widget(dialog, "mouse_speed"), "The speed of your mouse in scratch mode. Use negative values to invert motion.");
	
	gtk_range_set_value(GTK_RANGE(lookup_widget(dialog, "stop_sense_cycles")), globals.sense_cycles);
	gtk_widget_set_tooltip_text(lookup_widget(dialog, "stop_sense_cycles"),"If there is no \"motion-event\" for x cycles, where x is the number of cycles you select here, terminatorX assumes mouse motion has stopped. For smaller buffer sizes (=> shorter cycle times) you might have to increase this value");	

	gtk_range_set_value(GTK_RANGE(lookup_widget(dialog, "vtt_inertia")), globals.vtt_inertia);
	gtk_widget_set_tooltip_text(lookup_widget(dialog, "vtt_inertia"),"This value defines how fast the turntables will adapt to the speed input - the higher this value, the longer it will take the turntable to actually reach the target speed.");	

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(lookup_widget(dialog, "filename_length")), globals.filename_length);

	/* User Interface */ 
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "startup_nagbox")), globals.show_nag);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "mainwin_tooltips")), globals.tooltips);
	
	switch (globals.button_type) {
		case BUTTON_TYPE_TEXT:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "buttons_text_only")), 1);
			break;
		
		case BUTTON_TYPE_ICON:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "buttons_icon_only")), 1);
			break;
		
		case BUTTON_TYPE_BOTH:
		default:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "buttons_text_and_icon")), 1);
	}
	
	gtk_range_set_value(GTK_RANGE(lookup_widget(dialog, "update_delay")), globals.update_delay);
	gtk_widget_set_tooltip_text(lookup_widget(dialog, "update_delay"), "How often to update the slow widgets.");	
	gtk_range_set_value(GTK_RANGE(lookup_widget(dialog, "update_idle")), globals.update_idle);
	gtk_widget_set_tooltip_text(lookup_widget(dialog, "update_idle"), "The update thread will idle for the selcted amount of milliseconds. If you want to have a more responsive display update reduce this value - if you have performance problems increase this value.");	
	gtk_range_set_value(GTK_RANGE(lookup_widget(dialog, "vumeter_decay")), globals.flash_response);
	gtk_widget_set_tooltip_text(lookup_widget(dialog, "vumeter_decay"), "Defines how fast the maximum values of the VU meters should be decayed.");	

	/* Audio Colors */
	int ctr=0;
	
	set_color_button(globals.wav_display_bg_focus, "wav_display_bg_focus");
	set_color_button(globals.wav_display_bg_no_focus, "wav_display_bg_no_focus");
	set_color_button(globals.wav_display_fg_focus, "wav_display_fg_focus");
	set_color_button(globals.wav_display_fg_no_focus, "wav_display_fg_no_focus");
	set_color_button(globals.wav_display_cursor, "wav_display_cursor");
	set_color_button(globals.wav_display_cursor_mute, "wav_display_cursor_mute");
	
	/* VU Colors */
	set_color_button(globals.vu_meter_bg, "vu_meter_bg");
	set_color_button(globals.vu_meter_loud, "vu_meter_loud");
	set_color_button(globals.vu_meter_normal, "vu_meter_normal");
	
	gtk_range_set_value(GTK_RANGE(lookup_widget(dialog, "vu_meter_border_intensity")), globals.vu_meter_border_intensity);
	
	/* Misc */
	gtk_entry_set_text(GTK_ENTRY(lookup_widget(dialog, "soundfile_editor")), globals.file_editor);
	gtk_entry_set_text(GTK_ENTRY(lookup_widget(dialog, "ladspa_rdf_path")), globals.lrdf_path);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "compress_set_files")), globals.compress_set_files);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "prelisten_enabled")), globals.prelis);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "reconnect_enabled")), globals.restore_midi_connections);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "quit_confirm")), globals.quit_confirm);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "force_nonrt_plugins")), globals.force_nonrt_plugins);	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog, "verbose_plugin_loading")), globals.verbose_plugin_loading);	
}

void create_options()
{
	opt_dialog=create_tx_options();
	tX_set_icon(opt_dialog);
	init_tx_options(opt_dialog);
	gtk_widget_show(opt_dialog);
}

void display_options()
{
	if (opt_dialog) {
		gdk_window_raise(gtk_widget_get_window(opt_dialog));	
	} else {
		create_options();
	}
}

GtkWidget *about=NULL;

void raise_about()
{
	if (about) gdk_window_raise(gtk_widget_get_window(about));
}


void destroy_about()
{
	if (about) {	
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
	GtkWidget *window;
	GtkWidget *btn;
	GtkWidget *box;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *sep;
	GtkWidget *text;
	GtkWidget *scroll;
	GtkWidget *iwid;
	GtkWidget *expander;
	
	/* Only raise the window if it's already open... */
	if (about)  {
		gdk_window_raise(gtk_widget_get_window(about));
		return;
	}
	
	/* Create the window... */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 5);
	gtk_window_set_title(GTK_WINDOW(window), "terminatorX - About");
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	gtk_window_set_resizable(GTK_WINDOW(window), nag ? TRUE: FALSE);

	GdkPixbuf *image=gdk_pixbuf_new_from_xpm_data((const char **)logo_xpm);
	iwid = gtk_image_new_from_pixbuf(image);
	
	if (nag) {
		GtkWidget *box=gtk_vbox_new(FALSE, 2);
		GtkWidget *box2=gtk_hbox_new(FALSE, 2);
		GtkWidget *label;
		
		gtk_container_add(GTK_CONTAINER(window), box);
		gtk_box_pack_start(GTK_BOX(box), iwid, WID_FIX);
		gtk_box_pack_start(GTK_BOX(box), box2, WID_FIX);
		
		label=gtk_label_new(PACKAGE" release "VERSION);
		gtk_box_pack_start(GTK_BOX(box2), label, WID_DYN);
		gtk_misc_set_alignment(GTK_MISC(label), 0.1, 0.5);
		gtk_widget_show(label);

		label=gtk_label_new("Copyright (C) 1999-2014 by Alexander König");
		gtk_box_pack_start(GTK_BOX(box2), label, WID_DYN);
		gtk_misc_set_alignment(GTK_MISC(label), 0.9, 0.5);
		gtk_widget_show(label);
		
		gtk_widget_show(box2);
		gtk_widget_show(box);
		gtk_widget_show(iwid);

		gtk_window_set_decorated(GTK_WINDOW(window), FALSE);		
		gtk_widget_show(window);
	} else {
		box=gtk_vbox_new(FALSE, 5);
		add_about_wid_fix(iwid);
		
		sep=gtk_hseparator_new();
		add_about_wid_fix(sep);
		
		label=gtk_label_new("This is "PACKAGE" release "VERSION".\nCopyright (C) 1999-2014 by Alexander König <alex@lisas.de>");

		gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
		add_about_wid_fix(label);

		sep=gtk_hseparator_new();
		add_about_wid_fix(sep);

		expander = gtk_expander_new("Build info");
		GtkWidget *exbox=gtk_vbox_new(FALSE, 2);
		gtk_container_add(GTK_CONTAINER(expander), exbox);
		gtk_widget_show(exbox);

		label=gtk_label_new("Compilation flags: "
#ifdef USE_SOX_INPUT
		" [sox]"
#endif
#ifdef USE_MPG123_INPUT
		" [mpg123]"
#endif
#ifdef USE_OGG123_INPUT
		" [ogg123]"
#endif
#ifdef USE_MAD_INPUT
		" [mad]"
#endif
#ifdef USE_VORBIS_INPUT
		" [vorbis]"
#endif
#ifdef USE_AUDIOFILE_INPUT
		" [audiofile]"
#endif
#ifdef USE_ALSA_MIDI_IN
		" [midi]"
#endif
#ifdef USE_OSS
		" [oss]"
#endif
#ifdef USE_ALSA
		" [alsa]"
#endif
#ifdef USE_JACK
		" [jack]"
#endif
#ifdef USE_LRDF
		" [lrdf]"
#endif
#ifdef USE_SCHEDULER
		" [rt]"
#endif
#ifdef USE_CAPABILITIES
		" [cap]"
#endif
		"");
	
		gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
		gtk_widget_set_size_request(label, 640, -1);
		gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
		gtk_container_add(GTK_CONTAINER(exbox), label);
		gtk_widget_show(label);
		add_about_wid_fix(expander);

#ifdef 	USE_SCHEDULER
		sep=gtk_hseparator_new();
		gtk_widget_show(sep);
		gtk_container_add(GTK_CONTAINER(exbox), sep);

		char buffer[4096];
		
		int policy=-1;
		struct sched_param parm;
		
		pthread_getschedparam(tX_engine::get_instance()->get_thread_id(), &policy, &parm);
		char prio_str[32]="";
		bool s_enabled = false;
		
		switch (policy) {
			case SCHED_OTHER:
				strcpy(prio_str, "SCHED_OTHER");
				break;
			
			case SCHED_RR:
				strcpy(prio_str, "SCHED_RR");
				break;
			
			case SCHED_FIFO:
				strcpy(prio_str, "SCHED_FIFO");
				s_enabled = true;
				break;
			
			default:
				sprintf(prio_str, "UNKOWN (%i)", policy);
		}
		
		sprintf(buffer, "Audio engine scheduling policy: %s.\nScheduling with realtime priority is %s.", prio_str, s_enabled ? "enabled" : "disabled");

		label=gtk_label_new(buffer);

		gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
		gtk_widget_show(label);
		gtk_container_add(GTK_CONTAINER(exbox), label);
#endif

		sep=gtk_hseparator_new();
		add_about_wid_fix(sep);

		expander = gtk_expander_new("License (GPL V2):");

		add_about_wid_fix(expander);

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
		gtk_widget_set_size_request(GTK_WIDGET(text), 640, 200);
		gtk_widget_show(text);		
		
		gtk_container_add(GTK_CONTAINER(expander), scroll);
		gtk_widget_show(scroll);		
		
		sep=gtk_hseparator_new();
		add_about_wid_fix(sep);

		btn=gtk_button_new_with_label("Close");
		add_about_wid_fix(btn);

		gtk_container_add(GTK_CONTAINER(window), box);
		gtk_widget_show(box);
		
		g_signal_connect(G_OBJECT(btn), "clicked", (GCallback) destroy_about, NULL);		
		g_signal_connect(G_OBJECT(window), "delete-event", (GCallback) destroy_about, NULL);		
	}
	gtk_widget_show(window);	
	tX_set_icon(window);
	
	while (gtk_events_pending()) gtk_main_iteration();
		
	about=window;
}

static GdkPixbuf *tX_window_icon=NULL;

void tX_set_icon(GtkWidget *widget)
{	
	if (!tX_window_icon) {
		tX_window_icon=gdk_pixbuf_new_from_inline(-1, tX_icon, FALSE, NULL);
	}

	gtk_window_set_icon(GTK_WINDOW(widget), tX_window_icon);
}
