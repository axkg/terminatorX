/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "tX_glade_callbacks.h"
#include "tX_glade_interface.h"
#include "tX_glade_support.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

GtkWidget*
create_tx_options (void)
{
  GtkWidget *tx_options;
  GtkWidget *dialog_vbox1;
  GtkWidget *notebook1;
  GtkWidget *table4;
  GtkWidget *label18;
  GtkWidget *hbox2;
  GtkWidget *oss_driver;
  GSList *oss_driver_group = NULL;
  GtkWidget *alsa_driver;
  GtkWidget *jack_driver;
  GtkWidget *label1;
  GtkWidget *table5;
  GtkWidget *label21;
  GtkWidget *label22;
  GtkWidget *label23;
  GtkWidget *label24;
  GtkWidget *oss_audio_device;
  GtkWidget *combo_entry2;
  GtkObject *oss_buffers_adj;
  GtkWidget *oss_buffers;
  GtkWidget *oss_buffersize;
  GtkWidget *oss_samplerate;
  GtkWidget *combo_entry3;
  GtkWidget *label15;
  GtkWidget *table6;
  GtkWidget *label27;
  GtkWidget *label29;
  GtkWidget *label30;
  GtkWidget *alsa_audio_device;
  GtkWidget *combo_entry4;
  GtkWidget *alsa_samplerate;
  GtkWidget *combo_entry5;
  GtkWidget *alsa_period_time;
  GtkWidget *label32;
  GtkWidget *alsa_buffer_time;
  GtkWidget *label16;
  GtkWidget *table1;
  GtkWidget *label5;
  GtkWidget *label6;
  GtkWidget *label7;
  GtkWidget *xinput_device;
  GtkWidget *combo_entry1;
  GtkWidget *mouse_speed;
  GtkWidget *stop_sense_cycles;
  GtkWidget *label25;
  GtkWidget *xinput_enable;
  GtkWidget *label4;
  GtkWidget *table2;
  GtkWidget *label8;
  GtkWidget *label9;
  GtkWidget *label10;
  GtkWidget *label11;
  GtkWidget *mainwin_tooltips;
  GtkWidget *update_idle;
  GtkWidget *update_delay;
  GtkWidget *vumeter_decay;
  GtkWidget *label14;
  GtkWidget *startup_nagbox;
  GtkWidget *label12;
  GtkWidget *hbox1;
  GtkWidget *buttons_text_and_icon;
  GSList *buttons_text_and_icon_group = NULL;
  GtkWidget *buttons_icon_only;
  GtkWidget *buttons_text_only;
  GtkWidget *label2;
  GtkWidget *table3;
  GtkWidget *label13;
  GtkWidget *soundfile_editor;
  GtkWidget *label26;
  GtkWidget *prelisten_enabled;
  GtkWidget *label31;
  GtkWidget *ladspa_rdf_path;
  GtkWidget *label33;
  GtkWidget *compress_set_files;
  GtkWidget *label3;
  GtkWidget *dialog_action_area1;
  GtkWidget *pref_cancel;
  GtkWidget *pref_apply;
  GtkWidget *pref_ok;
  GtkTooltips *tooltips;

  tooltips = gtk_tooltips_new ();

  tx_options = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (tx_options), "terminatorX: Preferences");

  dialog_vbox1 = GTK_DIALOG (tx_options)->vbox;
  gtk_widget_show (dialog_vbox1);

  notebook1 = gtk_notebook_new ();
  gtk_widget_show (notebook1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), notebook1, TRUE, TRUE, 0);

  table4 = gtk_table_new (1, 2, FALSE);
  gtk_widget_show (table4);
  gtk_container_add (GTK_CONTAINER (notebook1), table4);
  gtk_container_set_border_width (GTK_CONTAINER (table4), 4);
  gtk_table_set_row_spacings (GTK_TABLE (table4), 2);
  gtk_table_set_col_spacings (GTK_TABLE (table4), 2);

  label18 = gtk_label_new ("Use Driver:");
  gtk_widget_show (label18);
  gtk_table_attach (GTK_TABLE (table4), label18, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label18), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label18), 0, 0.5);

  hbox2 = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox2);
  gtk_table_attach (GTK_TABLE (table4), hbox2, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  oss_driver = gtk_radio_button_new_with_mnemonic (NULL, "OSS");
  gtk_widget_show (oss_driver);
  gtk_box_pack_start (GTK_BOX (hbox2), oss_driver, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, oss_driver, "Use the OSS (Open Sound System) driver for audio output.", NULL);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (oss_driver), oss_driver_group);
  oss_driver_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (oss_driver));

  alsa_driver = gtk_radio_button_new_with_mnemonic (NULL, "ALSA");
  gtk_widget_show (alsa_driver);
  gtk_box_pack_start (GTK_BOX (hbox2), alsa_driver, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, alsa_driver, "Use the ALSA (Advanced Linux Sound System) driver for audio output.", NULL);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (alsa_driver), oss_driver_group);
  oss_driver_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (alsa_driver));

  jack_driver = gtk_radio_button_new_with_mnemonic (NULL, "JACK");
  gtk_widget_show (jack_driver);
  gtk_box_pack_start (GTK_BOX (hbox2), jack_driver, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, jack_driver, "Use the JACK (JACK Audio Connection Kit) driver for audio output.", NULL);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (jack_driver), oss_driver_group);
  oss_driver_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (jack_driver));

  label1 = gtk_label_new ("Audio");
  gtk_widget_show (label1);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), label1);
  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);

  table5 = gtk_table_new (4, 2, FALSE);
  gtk_widget_show (table5);
  gtk_container_add (GTK_CONTAINER (notebook1), table5);
  gtk_container_set_border_width (GTK_CONTAINER (table5), 4);
  gtk_table_set_row_spacings (GTK_TABLE (table5), 2);
  gtk_table_set_col_spacings (GTK_TABLE (table5), 2);

  label21 = gtk_label_new ("Audio Device:");
  gtk_widget_show (label21);
  gtk_table_attach (GTK_TABLE (table5), label21, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label21), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label21), 0, 0.5);

  label22 = gtk_label_new ("No. of Buffers:");
  gtk_widget_show (label22);
  gtk_table_attach (GTK_TABLE (table5), label22, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label22), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label22), 0, 0.5);

  label23 = gtk_label_new ("Buffersize (2^x):");
  gtk_widget_show (label23);
  gtk_table_attach (GTK_TABLE (table5), label23, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label23), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label23), 0, 0.5);

  label24 = gtk_label_new ("Samplerate (Hz):");
  gtk_widget_show (label24);
  gtk_table_attach (GTK_TABLE (table5), label24, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label24), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label24), 0, 0.5);

  oss_audio_device = gtk_combo_new ();
  g_object_set_data (G_OBJECT (GTK_COMBO (oss_audio_device)->popwin),
                     "GladeParentKey", oss_audio_device);
  gtk_widget_show (oss_audio_device);
  gtk_table_attach (GTK_TABLE (table5), oss_audio_device, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  combo_entry2 = GTK_COMBO (oss_audio_device)->entry;
  gtk_widget_show (combo_entry2);
  gtk_tooltips_set_tip (tooltips, combo_entry2, "Select the audiodevice you want terminatorX to send its output to.", NULL);

  oss_buffers_adj = gtk_adjustment_new (2, 2, 5, 1, 10, 10);
  oss_buffers = gtk_spin_button_new (GTK_ADJUSTMENT (oss_buffers_adj), 1, 0);
  gtk_widget_show (oss_buffers);
  gtk_table_attach (GTK_TABLE (table5), oss_buffers, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, oss_buffers, "Sets the number of kernel level audio buffers. Actually most systems should run just fine with two.", NULL);

  oss_buffersize = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (9, 8, 16, 1, 1, 1)));
  gtk_widget_show (oss_buffersize);
  gtk_table_attach (GTK_TABLE (table5), oss_buffersize, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_scale_set_digits (GTK_SCALE (oss_buffersize), 0);

  oss_samplerate = gtk_combo_new ();
  g_object_set_data (G_OBJECT (GTK_COMBO (oss_samplerate)->popwin),
                     "GladeParentKey", oss_samplerate);
  gtk_widget_show (oss_samplerate);
  gtk_table_attach (GTK_TABLE (table5), oss_samplerate, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  combo_entry3 = GTK_COMBO (oss_samplerate)->entry;
  gtk_widget_show (combo_entry3);
  gtk_tooltips_set_tip (tooltips, combo_entry3, "Select the sampling to use for this audio device - the higher the better quality. Note that not all sampling rates are supported by all audio devices.", NULL);

  label15 = gtk_label_new ("Audio: OSS");
  gtk_widget_show (label15);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), label15);
  gtk_label_set_justify (GTK_LABEL (label15), GTK_JUSTIFY_LEFT);

  table6 = gtk_table_new (4, 2, FALSE);
  gtk_widget_show (table6);
  gtk_container_add (GTK_CONTAINER (notebook1), table6);
  gtk_container_set_border_width (GTK_CONTAINER (table6), 4);
  gtk_table_set_row_spacings (GTK_TABLE (table6), 2);
  gtk_table_set_col_spacings (GTK_TABLE (table6), 2);

  label27 = gtk_label_new ("Audio Device:");
  gtk_widget_show (label27);
  gtk_table_attach (GTK_TABLE (table6), label27, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label27), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label27), 0, 0.5);

  label29 = gtk_label_new ("Period Time (ms):");
  gtk_widget_show (label29);
  gtk_table_attach (GTK_TABLE (table6), label29, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label29), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label29), 0, 0.5);

  label30 = gtk_label_new ("Samplerate (Hz):");
  gtk_widget_show (label30);
  gtk_table_attach (GTK_TABLE (table6), label30, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label30), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label30), 0, 0.5);

  alsa_audio_device = gtk_combo_new ();
  g_object_set_data (G_OBJECT (GTK_COMBO (alsa_audio_device)->popwin),
                     "GladeParentKey", alsa_audio_device);
  gtk_widget_show (alsa_audio_device);
  gtk_table_attach (GTK_TABLE (table6), alsa_audio_device, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  combo_entry4 = GTK_COMBO (alsa_audio_device)->entry;
  gtk_widget_show (combo_entry4);

  alsa_samplerate = gtk_combo_new ();
  g_object_set_data (G_OBJECT (GTK_COMBO (alsa_samplerate)->popwin),
                     "GladeParentKey", alsa_samplerate);
  gtk_widget_show (alsa_samplerate);
  gtk_table_attach (GTK_TABLE (table6), alsa_samplerate, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  combo_entry5 = GTK_COMBO (alsa_samplerate)->entry;
  gtk_widget_show (combo_entry5);

  alsa_period_time = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (29, 10, 500, 1, 10, 10)));
  gtk_widget_show (alsa_period_time);
  gtk_table_attach (GTK_TABLE (table6), alsa_period_time, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_scale_set_digits (GTK_SCALE (alsa_period_time), 0);

  label32 = gtk_label_new ("Buffer Time (ms):");
  gtk_widget_show (label32);
  gtk_table_attach (GTK_TABLE (table6), label32, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label32), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label32), 0, 0.5);

  alsa_buffer_time = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (50, 10, 500, 1, 10, 10)));
  gtk_widget_show (alsa_buffer_time);
  gtk_table_attach (GTK_TABLE (table6), alsa_buffer_time, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_scale_set_digits (GTK_SCALE (alsa_buffer_time), 0);

  label16 = gtk_label_new ("Audio: ALSA");
  gtk_widget_show (label16);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 2), label16);
  gtk_label_set_justify (GTK_LABEL (label16), GTK_JUSTIFY_LEFT);

  table1 = gtk_table_new (4, 2, FALSE);
  gtk_widget_show (table1);
  gtk_container_add (GTK_CONTAINER (notebook1), table1);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 4);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 2);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 2);

  label5 = gtk_label_new ("XInput Device:");
  gtk_widget_show (label5);
  gtk_table_attach (GTK_TABLE (table1), label5, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label5), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label5), 0, 0.5);

  label6 = gtk_label_new ("Mouse Speed:");
  gtk_widget_show (label6);
  gtk_table_attach (GTK_TABLE (table1), label6, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label6), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label6), 0, 0.5);

  label7 = gtk_label_new ("Stop-Sense-Cycles:");
  gtk_widget_show (label7);
  gtk_table_attach (GTK_TABLE (table1), label7, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label7), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label7), 0, 0.5);

  xinput_device = gtk_combo_new ();
  g_object_set_data (G_OBJECT (GTK_COMBO (xinput_device)->popwin),
                     "GladeParentKey", xinput_device);
  gtk_widget_show (xinput_device);
  gtk_table_attach (GTK_TABLE (table1), xinput_device, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  combo_entry1 = GTK_COMBO (xinput_device)->entry;
  gtk_widget_show (combo_entry1);
  gtk_tooltips_set_tip (tooltips, combo_entry1, "Select the input device to use when XInput is enabled. Note: do not use this option if you plan on using your default device (standard mouse).", NULL);

  mouse_speed = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, -10, 10, 0.5, 0.1, 0.1)));
  gtk_widget_show (mouse_speed);
  gtk_table_attach (GTK_TABLE (table1), mouse_speed, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  stop_sense_cycles = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (10, 1, 150, 5, 1, 1)));
  gtk_widget_show (stop_sense_cycles);
  gtk_table_attach (GTK_TABLE (table1), stop_sense_cycles, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_scale_set_digits (GTK_SCALE (stop_sense_cycles), 0);

  label25 = gtk_label_new ("XInput:");
  gtk_widget_show (label25);
  gtk_table_attach (GTK_TABLE (table1), label25, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label25), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label25), 0, 0.5);

  xinput_enable = gtk_check_button_new_with_mnemonic ("Enabled");
  gtk_widget_show (xinput_enable);
  gtk_table_attach (GTK_TABLE (table1), xinput_enable, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, xinput_enable, "CAREFUL! Enable this *only* if you want to use an input device than your default X-Pointer (yes, your mouse ;). You have to select your desired device as well. Selecting the default mouse pointer will crash terminatorX so if you want to use that keep this option disabled.", NULL);

  label4 = gtk_label_new ("Input");
  gtk_widget_show (label4);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 3), label4);
  gtk_label_set_justify (GTK_LABEL (label4), GTK_JUSTIFY_LEFT);

  table2 = gtk_table_new (6, 2, FALSE);
  gtk_widget_show (table2);
  gtk_container_add (GTK_CONTAINER (notebook1), table2);
  gtk_container_set_border_width (GTK_CONTAINER (table2), 4);
  gtk_table_set_row_spacings (GTK_TABLE (table2), 2);
  gtk_table_set_col_spacings (GTK_TABLE (table2), 2);

  label8 = gtk_label_new ("Main Window Tooltips:");
  gtk_widget_show (label8);
  gtk_table_attach (GTK_TABLE (table2), label8, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label8), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label8), 0, 0.5);

  label9 = gtk_label_new ("Update Idle (ms):");
  gtk_widget_show (label9);
  gtk_table_attach (GTK_TABLE (table2), label9, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label9), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label9), 0, 0.5);

  label10 = gtk_label_new ("Update Delay (cycles):");
  gtk_widget_show (label10);
  gtk_table_attach (GTK_TABLE (table2), label10, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label10), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label10), 0, 0.5);

  label11 = gtk_label_new ("VU Meter Decay:");
  gtk_widget_show (label11);
  gtk_table_attach (GTK_TABLE (table2), label11, 0, 1, 5, 6,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label11), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label11), 0, 0.5);

  mainwin_tooltips = gtk_check_button_new_with_mnemonic ("Enabled");
  gtk_widget_show (mainwin_tooltips);
  gtk_table_attach (GTK_TABLE (table2), mainwin_tooltips, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, mainwin_tooltips, "Enable tooltips for the terminatorX main window.", NULL);

  update_idle = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (11, 1, 100, 1, 10, 10)));
  gtk_widget_show (update_idle);
  gtk_table_attach (GTK_TABLE (table2), update_idle, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_scale_set_digits (GTK_SCALE (update_idle), 0);

  update_delay = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 15, 1, 10, 10)));
  gtk_widget_show (update_delay);
  gtk_table_attach (GTK_TABLE (table2), update_delay, 1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  vumeter_decay = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0.8, 0.8, 0.99, 0.01, 0.01, 0.001)));
  gtk_widget_show (vumeter_decay);
  gtk_table_attach (GTK_TABLE (table2), vumeter_decay, 1, 2, 5, 6,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  label14 = gtk_label_new ("Startup-Nagbox:");
  gtk_widget_show (label14);
  gtk_table_attach (GTK_TABLE (table2), label14, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label14), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label14), 0, 0.5);

  startup_nagbox = gtk_check_button_new_with_mnemonic ("Enabled");
  gtk_widget_show (startup_nagbox);
  gtk_table_attach (GTK_TABLE (table2), startup_nagbox, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, startup_nagbox, "Display nagbox on startup while loading data.", NULL);

  label12 = gtk_label_new ("Buttons:");
  gtk_widget_show (label12);
  gtk_table_attach (GTK_TABLE (table2), label12, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label12), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label12), 0, 0.5);

  hbox1 = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox1);
  gtk_table_attach (GTK_TABLE (table2), hbox1, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  buttons_text_and_icon = gtk_radio_button_new_with_mnemonic (NULL, "Text + Icon");
  gtk_widget_show (buttons_text_and_icon);
  gtk_box_pack_start (GTK_BOX (hbox1), buttons_text_and_icon, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (buttons_text_and_icon), buttons_text_and_icon_group);
  buttons_text_and_icon_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttons_text_and_icon));

  buttons_icon_only = gtk_radio_button_new_with_mnemonic (NULL, "Icon Only");
  gtk_widget_show (buttons_icon_only);
  gtk_box_pack_start (GTK_BOX (hbox1), buttons_icon_only, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (buttons_icon_only), buttons_text_and_icon_group);
  buttons_text_and_icon_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttons_icon_only));

  buttons_text_only = gtk_radio_button_new_with_mnemonic (NULL, "Text Only");
  gtk_widget_show (buttons_text_only);
  gtk_box_pack_start (GTK_BOX (hbox1), buttons_text_only, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (buttons_text_only), buttons_text_and_icon_group);
  buttons_text_and_icon_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttons_text_only));

  label2 = gtk_label_new ("User Interface");
  gtk_widget_show (label2);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 4), label2);
  gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);

  table3 = gtk_table_new (4, 2, FALSE);
  gtk_widget_show (table3);
  gtk_container_add (GTK_CONTAINER (notebook1), table3);
  gtk_container_set_border_width (GTK_CONTAINER (table3), 4);
  gtk_table_set_row_spacings (GTK_TABLE (table3), 2);
  gtk_table_set_col_spacings (GTK_TABLE (table3), 2);

  label13 = gtk_label_new ("Soundfile Editor:");
  gtk_widget_show (label13);
  gtk_table_attach (GTK_TABLE (table3), label13, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label13), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label13), 0, 0.5);

  soundfile_editor = gtk_entry_new ();
  gtk_widget_show (soundfile_editor);
  gtk_table_attach (GTK_TABLE (table3), soundfile_editor, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, soundfile_editor, "Enter the command to run your favourite soundfile editor. It will be started when you choose \"Edit File\" from the turntable's file menu.", NULL);

  label26 = gtk_label_new ("\"Pre-Listen\" to soundfiles:");
  gtk_widget_show (label26);
  gtk_table_attach (GTK_TABLE (table3), label26, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label26), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label26), 0, 0.5);

  prelisten_enabled = gtk_check_button_new_with_mnemonic ("Enabled");
  gtk_widget_show (prelisten_enabled);
  gtk_table_attach (GTK_TABLE (table3), prelisten_enabled, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, prelisten_enabled, "When enabled soundfiles will be playedback when selected in a file dialog (before loading them).", NULL);

  label31 = gtk_label_new ("LADSPA RDF Path:");
  gtk_widget_show (label31);
  gtk_table_attach (GTK_TABLE (table3), label31, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label31), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label31), 0, 0.5);

  ladspa_rdf_path = gtk_entry_new ();
  gtk_widget_show (ladspa_rdf_path);
  gtk_table_attach (GTK_TABLE (table3), ladspa_rdf_path, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  label33 = gtk_label_new ("Compress set files:");
  gtk_widget_show (label33);
  gtk_table_attach (GTK_TABLE (table3), label33, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label33), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label33), 0, 0.5);

  compress_set_files = gtk_check_button_new_with_mnemonic ("Enabled");
  gtk_widget_show (compress_set_files);
  gtk_table_attach (GTK_TABLE (table3), compress_set_files, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  label3 = gtk_label_new ("Misc");
  gtk_widget_show (label3);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 5), label3);
  gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);

  dialog_action_area1 = GTK_DIALOG (tx_options)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  pref_cancel = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (pref_cancel);
  gtk_dialog_add_action_widget (GTK_DIALOG (tx_options), pref_cancel, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (pref_cancel, GTK_CAN_DEFAULT);

  pref_apply = gtk_button_new_from_stock ("gtk-apply");
  gtk_widget_show (pref_apply);
  gtk_dialog_add_action_widget (GTK_DIALOG (tx_options), pref_apply, GTK_RESPONSE_APPLY);
  GTK_WIDGET_SET_FLAGS (pref_apply, GTK_CAN_DEFAULT);

  pref_ok = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (pref_ok);
  gtk_dialog_add_action_widget (GTK_DIALOG (tx_options), pref_ok, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (pref_ok, GTK_CAN_DEFAULT);

  g_signal_connect ((gpointer) tx_options, "destroy",
                    G_CALLBACK (on_tx_options_destroy),
                    NULL);
  g_signal_connect_swapped ((gpointer) alsa_buffer_time, "value_changed",
                            G_CALLBACK (on_alsa_buffer_time_value_changed),
                            GTK_OBJECT (alsa_period_time));
  g_signal_connect ((gpointer) pref_cancel, "clicked",
                    G_CALLBACK (on_pref_cancel_clicked),
                    NULL);
  g_signal_connect ((gpointer) pref_apply, "clicked",
                    G_CALLBACK (on_pref_apply_clicked),
                    NULL);
  g_signal_connect ((gpointer) pref_ok, "clicked",
                    G_CALLBACK (on_pref_ok_clicked),
                    NULL);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (tx_options, tx_options, "tx_options");
  GLADE_HOOKUP_OBJECT_NO_REF (tx_options, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (tx_options, notebook1, "notebook1");
  GLADE_HOOKUP_OBJECT (tx_options, table4, "table4");
  GLADE_HOOKUP_OBJECT (tx_options, label18, "label18");
  GLADE_HOOKUP_OBJECT (tx_options, hbox2, "hbox2");
  GLADE_HOOKUP_OBJECT (tx_options, oss_driver, "oss_driver");
  GLADE_HOOKUP_OBJECT (tx_options, alsa_driver, "alsa_driver");
  GLADE_HOOKUP_OBJECT (tx_options, jack_driver, "jack_driver");
  GLADE_HOOKUP_OBJECT (tx_options, label1, "label1");
  GLADE_HOOKUP_OBJECT (tx_options, table5, "table5");
  GLADE_HOOKUP_OBJECT (tx_options, label21, "label21");
  GLADE_HOOKUP_OBJECT (tx_options, label22, "label22");
  GLADE_HOOKUP_OBJECT (tx_options, label23, "label23");
  GLADE_HOOKUP_OBJECT (tx_options, label24, "label24");
  GLADE_HOOKUP_OBJECT (tx_options, oss_audio_device, "oss_audio_device");
  GLADE_HOOKUP_OBJECT (tx_options, combo_entry2, "combo_entry2");
  GLADE_HOOKUP_OBJECT (tx_options, oss_buffers, "oss_buffers");
  GLADE_HOOKUP_OBJECT (tx_options, oss_buffersize, "oss_buffersize");
  GLADE_HOOKUP_OBJECT (tx_options, oss_samplerate, "oss_samplerate");
  GLADE_HOOKUP_OBJECT (tx_options, combo_entry3, "combo_entry3");
  GLADE_HOOKUP_OBJECT (tx_options, label15, "label15");
  GLADE_HOOKUP_OBJECT (tx_options, table6, "table6");
  GLADE_HOOKUP_OBJECT (tx_options, label27, "label27");
  GLADE_HOOKUP_OBJECT (tx_options, label29, "label29");
  GLADE_HOOKUP_OBJECT (tx_options, label30, "label30");
  GLADE_HOOKUP_OBJECT (tx_options, alsa_audio_device, "alsa_audio_device");
  GLADE_HOOKUP_OBJECT (tx_options, combo_entry4, "combo_entry4");
  GLADE_HOOKUP_OBJECT (tx_options, alsa_samplerate, "alsa_samplerate");
  GLADE_HOOKUP_OBJECT (tx_options, combo_entry5, "combo_entry5");
  GLADE_HOOKUP_OBJECT (tx_options, alsa_period_time, "alsa_period_time");
  GLADE_HOOKUP_OBJECT (tx_options, label32, "label32");
  GLADE_HOOKUP_OBJECT (tx_options, alsa_buffer_time, "alsa_buffer_time");
  GLADE_HOOKUP_OBJECT (tx_options, label16, "label16");
  GLADE_HOOKUP_OBJECT (tx_options, table1, "table1");
  GLADE_HOOKUP_OBJECT (tx_options, label5, "label5");
  GLADE_HOOKUP_OBJECT (tx_options, label6, "label6");
  GLADE_HOOKUP_OBJECT (tx_options, label7, "label7");
  GLADE_HOOKUP_OBJECT (tx_options, xinput_device, "xinput_device");
  GLADE_HOOKUP_OBJECT (tx_options, combo_entry1, "combo_entry1");
  GLADE_HOOKUP_OBJECT (tx_options, mouse_speed, "mouse_speed");
  GLADE_HOOKUP_OBJECT (tx_options, stop_sense_cycles, "stop_sense_cycles");
  GLADE_HOOKUP_OBJECT (tx_options, label25, "label25");
  GLADE_HOOKUP_OBJECT (tx_options, xinput_enable, "xinput_enable");
  GLADE_HOOKUP_OBJECT (tx_options, label4, "label4");
  GLADE_HOOKUP_OBJECT (tx_options, table2, "table2");
  GLADE_HOOKUP_OBJECT (tx_options, label8, "label8");
  GLADE_HOOKUP_OBJECT (tx_options, label9, "label9");
  GLADE_HOOKUP_OBJECT (tx_options, label10, "label10");
  GLADE_HOOKUP_OBJECT (tx_options, label11, "label11");
  GLADE_HOOKUP_OBJECT (tx_options, mainwin_tooltips, "mainwin_tooltips");
  GLADE_HOOKUP_OBJECT (tx_options, update_idle, "update_idle");
  GLADE_HOOKUP_OBJECT (tx_options, update_delay, "update_delay");
  GLADE_HOOKUP_OBJECT (tx_options, vumeter_decay, "vumeter_decay");
  GLADE_HOOKUP_OBJECT (tx_options, label14, "label14");
  GLADE_HOOKUP_OBJECT (tx_options, startup_nagbox, "startup_nagbox");
  GLADE_HOOKUP_OBJECT (tx_options, label12, "label12");
  GLADE_HOOKUP_OBJECT (tx_options, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (tx_options, buttons_text_and_icon, "buttons_text_and_icon");
  GLADE_HOOKUP_OBJECT (tx_options, buttons_icon_only, "buttons_icon_only");
  GLADE_HOOKUP_OBJECT (tx_options, buttons_text_only, "buttons_text_only");
  GLADE_HOOKUP_OBJECT (tx_options, label2, "label2");
  GLADE_HOOKUP_OBJECT (tx_options, table3, "table3");
  GLADE_HOOKUP_OBJECT (tx_options, label13, "label13");
  GLADE_HOOKUP_OBJECT (tx_options, soundfile_editor, "soundfile_editor");
  GLADE_HOOKUP_OBJECT (tx_options, label26, "label26");
  GLADE_HOOKUP_OBJECT (tx_options, prelisten_enabled, "prelisten_enabled");
  GLADE_HOOKUP_OBJECT (tx_options, label31, "label31");
  GLADE_HOOKUP_OBJECT (tx_options, ladspa_rdf_path, "ladspa_rdf_path");
  GLADE_HOOKUP_OBJECT (tx_options, label33, "label33");
  GLADE_HOOKUP_OBJECT (tx_options, compress_set_files, "compress_set_files");
  GLADE_HOOKUP_OBJECT (tx_options, label3, "label3");
  GLADE_HOOKUP_OBJECT_NO_REF (tx_options, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (tx_options, pref_cancel, "pref_cancel");
  GLADE_HOOKUP_OBJECT (tx_options, pref_apply, "pref_apply");
  GLADE_HOOKUP_OBJECT (tx_options, pref_ok, "pref_ok");
  GLADE_HOOKUP_OBJECT_NO_REF (tx_options, tooltips, "tooltips");

  return tx_options;
}

GtkWidget*
create_tx_adjust (void)
{
  GtkWidget *tx_adjust;
  GtkWidget *dialog_vbox2;
  GtkWidget *vbox1;
  GtkWidget *label34;
  GtkWidget *hbox3;
  GtkWidget *label35;
  GtkObject *master_cycles_adj;
  GtkWidget *master_cycles;
  GtkWidget *label36;
  GtkObject *cycles_adj;
  GtkWidget *cycles;
  GtkWidget *create_event;
  GtkWidget *dialog_action_area2;
  GtkWidget *cancel;
  GtkWidget *ok;

  tx_adjust = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (tx_adjust), "Compute Pitch");

  dialog_vbox2 = GTK_DIALOG (tx_adjust)->vbox;
  gtk_widget_show (dialog_vbox2);

  vbox1 = gtk_vbox_new (FALSE, 4);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox2), vbox1, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 4);

  label34 = gtk_label_new ("To compute the pitch value, please specify with how many cycles of the master turntable this turntable should be re-triggered:");
  gtk_widget_show (label34);
  gtk_box_pack_start (GTK_BOX (vbox1), label34, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label34), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (label34), TRUE);

  hbox3 = gtk_hbox_new (FALSE, 4);
  gtk_widget_show (hbox3);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox3, FALSE, FALSE, 0);

  label35 = gtk_label_new ("Master Cycles:");
  gtk_widget_show (label35);
  gtk_box_pack_start (GTK_BOX (hbox3), label35, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label35), GTK_JUSTIFY_LEFT);

  master_cycles_adj = gtk_adjustment_new (1, 1, 100, 1, 10, 10);
  master_cycles = gtk_spin_button_new (GTK_ADJUSTMENT (master_cycles_adj), 1, 0);
  gtk_widget_show (master_cycles);
  gtk_box_pack_start (GTK_BOX (hbox3), master_cycles, TRUE, TRUE, 0);

  label36 = gtk_label_new ("Cycles:");
  gtk_widget_show (label36);
  gtk_box_pack_start (GTK_BOX (hbox3), label36, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label36), GTK_JUSTIFY_LEFT);

  cycles_adj = gtk_adjustment_new (1, 1, 100, 1, 10, 10);
  cycles = gtk_spin_button_new (GTK_ADJUSTMENT (cycles_adj), 1, 0);
  gtk_widget_show (cycles);
  gtk_box_pack_start (GTK_BOX (hbox3), cycles, TRUE, TRUE, 0);

  create_event = gtk_check_button_new_with_mnemonic ("Record a sequencer event");
  gtk_widget_show (create_event);
  gtk_box_pack_start (GTK_BOX (vbox1), create_event, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (create_event), TRUE);

  dialog_action_area2 = GTK_DIALOG (tx_adjust)->action_area;
  gtk_widget_show (dialog_action_area2);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area2), GTK_BUTTONBOX_END);

  cancel = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (cancel);
  gtk_dialog_add_action_widget (GTK_DIALOG (tx_adjust), cancel, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (cancel, GTK_CAN_DEFAULT);

  ok = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (ok);
  gtk_dialog_add_action_widget (GTK_DIALOG (tx_adjust), ok, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (ok, GTK_CAN_DEFAULT);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (tx_adjust, tx_adjust, "tx_adjust");
  GLADE_HOOKUP_OBJECT_NO_REF (tx_adjust, dialog_vbox2, "dialog_vbox2");
  GLADE_HOOKUP_OBJECT (tx_adjust, vbox1, "vbox1");
  GLADE_HOOKUP_OBJECT (tx_adjust, label34, "label34");
  GLADE_HOOKUP_OBJECT (tx_adjust, hbox3, "hbox3");
  GLADE_HOOKUP_OBJECT (tx_adjust, label35, "label35");
  GLADE_HOOKUP_OBJECT (tx_adjust, master_cycles, "master_cycles");
  GLADE_HOOKUP_OBJECT (tx_adjust, label36, "label36");
  GLADE_HOOKUP_OBJECT (tx_adjust, cycles, "cycles");
  GLADE_HOOKUP_OBJECT (tx_adjust, create_event, "create_event");
  GLADE_HOOKUP_OBJECT_NO_REF (tx_adjust, dialog_action_area2, "dialog_action_area2");
  GLADE_HOOKUP_OBJECT (tx_adjust, cancel, "cancel");
  GLADE_HOOKUP_OBJECT (tx_adjust, ok, "ok");

  return tx_adjust;
}

