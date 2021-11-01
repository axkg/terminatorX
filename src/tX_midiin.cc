/*
  terminatorX - realtime audio scratching software
  Copyright (C) 2002 Arthur Peters

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

  File: tX_midiin.cc

  Description: Implements MIDI input to control turntable parameters.
*/

#include "tX_midiin.h"
#include "tX_dialog.h"
#include "tX_maingui.h"
#include "tX_ui_interface.h"
#include "tX_ui_support.h"
#include "tX_vtt.h"

#ifdef USE_ALSA_MIDI_IN
#include "tX_engine.h"
#include "tX_global.h"
#include <iostream>

using namespace std;

static gboolean midi_callback(GIOChannel* source, GIOCondition condition, gpointer data) {
    tX_midiin* midi = (tX_midiin*)data;
    midi->check_event();

    return TRUE;
}

tX_midiin::tX_midiin() {
    is_open = false;
    sp_to_learn = NULL;
    learn_dialog = NULL;

    if (snd_seq_open(&ALSASeqHandle, "default", SND_SEQ_OPEN_INPUT, 0) < 0) {
        tX_error("tX_midiin(): failed to open the default sequencer device.");
        return;
    }
    snd_seq_set_client_name(ALSASeqHandle, "terminatorX");
    portid = snd_seq_create_simple_port(ALSASeqHandle,
        "Control Input",
        SND_SEQ_PORT_CAP_WRITE
            | SND_SEQ_PORT_CAP_SUBS_WRITE,
        SND_SEQ_PORT_TYPE_APPLICATION);
    if (portid < 0) {
        tX_error("tX_midiin(): error creating sequencer port.");
        return;
    }

    snd_seq_nonblock(ALSASeqHandle, 1);

    struct pollfd fds[32];

    int res = snd_seq_poll_descriptors(ALSASeqHandle, fds, 32, POLLIN);

    if (res != 1) {
        tX_error("Failed to poll ALSA descriptors: %i.\n", res);
    }

    GIOChannel* ioc = g_io_channel_unix_new(fds[0].fd);
    g_io_add_watch(ioc, (GIOCondition)(G_IO_IN), midi_callback, (gpointer)this);
    g_io_channel_unref(ioc);

    is_open = true;

    tX_debug("tX_midiin(): sequencer successfully opened.");
}

tX_midiin::~tX_midiin() {
    if (is_open) {
        snd_seq_close(ALSASeqHandle);
        tX_debug("tX_midiin(): sequencer closed.");
    }
}

int tX_midiin::check_event() {
    snd_seq_event_t* ev;

    while (snd_seq_event_input(ALSASeqHandle, &ev) != -EAGAIN) {

        // MidiEvent::type MessageType=MidiEvent::NONE;
        // int Volume=0,Note=0,EventDevice=0;
        tX_midievent event;
        event.is_noteon = false;
        bool event_usable = true;

        switch (ev->type) {
        case SND_SEQ_EVENT_CONTROLLER:
            event.type = tX_midievent::CC;
            event.number = ev->data.control.param;
            event.value = ev->data.control.value / 127.0;
            event.channel = ev->data.control.channel;
            break;
        case SND_SEQ_EVENT_PITCHBEND:
            event.type = tX_midievent::PITCHBEND;
            event.number = ev->data.control.param;
            event.value = (ev->data.control.value + 8191.0) / 16382.0; // 127.0;
            event.channel = ev->data.control.channel;
            break;
        case SND_SEQ_EVENT_CONTROL14:
            event.type = tX_midievent::CC14;
            event.number = ev->data.control.param;
            event.value = ev->data.control.value / 16383.0;
            event.channel = ev->data.control.channel;
            break;
        case SND_SEQ_EVENT_REGPARAM:
            event.type = tX_midievent::RPN;
            event.number = ev->data.control.param;
            event.value = ev->data.control.value / 16383.0;
            event.channel = ev->data.control.channel;
            break;
        case SND_SEQ_EVENT_NONREGPARAM:
            event.type = tX_midievent::NRPN;
            event.number = ev->data.control.param;
            event.value = ev->data.control.value / 16383.0;
            event.channel = ev->data.control.channel;
            break;
        case SND_SEQ_EVENT_NOTEON:
            event.type = tX_midievent::NOTE;
            event.number = ev->data.note.note;
            event.value = ev->data.note.velocity / 127.0;
            event.channel = ev->data.note.channel;

            event.is_noteon = true;
            if (event.value == 0)
                event.is_noteon = false;
            break;
        case SND_SEQ_EVENT_NOTEOFF:
            event.type = tX_midievent::NOTE;
            event.number = ev->data.note.note;
            event.value = ev->data.note.velocity / 127.0;
            event.channel = ev->data.note.channel;

            event.is_noteon = false;
            break;
        default:
            event_usable = false;
        }

        snd_seq_free_event(ev);

        if (event_usable) {
            if (event.channel < 0 || event.channel > 15) {
                tX_error("tX_midiin::check_event(): invaild event channel %i.", event.channel);
                return -1;
            }

            if (sp_to_learn) {
                sp_to_learn->bound_midi_event = event;
                sp_to_learn = NULL;

                if (learn_dialog) {
                    gtk_widget_destroy(learn_dialog);
                    learn_dialog = NULL;
                }
            } else {
                // This should be solved with a hash table. Possibly.

                list<tX_seqpar*>::iterator sp;

                for (sp = tX_seqpar::all->begin(); sp != tX_seqpar::all->end(); sp++) {
                    if ((*sp)->bound_midi_event.type_matches(event)) {
                        (*sp)->handle_midi_input(event);
                    }
                }
            }

            last_event = event;
        }
    }
    return 1;
}

void tX_midiin::configure_bindings(vtt_class* vtt) {
    list<tX_seqpar*>::iterator sp;

    GType types[3] = { G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER };
    GtkListStore* model = gtk_list_store_newv(3, types);
    GtkTreeIter iter;
    char tempstr[128];

    for (sp = tX_seqpar::all->begin(); sp != tX_seqpar::all->end(); sp++) {
        if (((*sp)->is_mappable) && ((*sp)->vtt) == (void*)vtt) {

            snprintf(tempstr, sizeof(tempstr), "Type: %d, Number: %d, Channel: %d",
                (*sp)->bound_midi_event.type, (*sp)->bound_midi_event.number,
                (*sp)->bound_midi_event.channel);

            gtk_list_store_append(model, &iter);
            gtk_list_store_set(model, &iter,
                0, (*sp)->get_name(),
                1, tempstr,
                2, (*sp),
                -1);
        }
    }

    // it will delete itself.
    new midi_binding_gui(GTK_TREE_MODEL(model), this);
}

tX_midiin::midi_binding_gui::midi_binding_gui(GtkTreeModel* _model, tX_midiin* _midi)
    : model(_model)
    , midi(_midi) {
    GtkWidget* hbox1;
    GtkWidget* scrolledwindow1;
    GtkWidget* vbox1;
    GtkWidget* label1;
    GtkWidget* frame1;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Configure MIDI Bindings");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 260);

    hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_widget_show(hbox1);
    gtk_container_add(GTK_CONTAINER(window), hbox1);
    gtk_container_set_border_width(GTK_CONTAINER(hbox1), 4);

    scrolledwindow1 = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_show(scrolledwindow1);
    gtk_box_pack_start(GTK_BOX(hbox1), scrolledwindow1, TRUE, TRUE, 0);

    parameter_treeview = gtk_tree_view_new_with_model(model);
    gtk_widget_show(parameter_treeview);
    gtk_container_add(GTK_CONTAINER(scrolledwindow1), parameter_treeview);

    GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(parameter_treeview),
        -1, "Parameter", renderer,
        "text", 0,
        NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(parameter_treeview),
        -1, "Event", renderer,
        "text", 1,
        NULL);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(parameter_treeview), TRUE);

    vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_show(vbox1);
    gtk_box_pack_start(GTK_BOX(hbox1), vbox1, FALSE, FALSE, 0);

    label1 = gtk_label_new("Selected MIDI Event:");
    gtk_widget_show(label1);
    gtk_box_pack_start(GTK_BOX(vbox1), label1, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label1), GTK_JUSTIFY_LEFT);

    frame1 = gtk_frame_new(NULL);
    gtk_widget_show(frame1);
    gtk_box_pack_start(GTK_BOX(vbox1), frame1, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(frame1), 2);
    gtk_frame_set_label_align(GTK_FRAME(frame1), 0, 0);
    gtk_frame_set_shadow_type(GTK_FRAME(frame1), GTK_SHADOW_IN);

    midi_event_info = gtk_label_new("Use a MIDI thing to select it.");
    gtk_widget_show(midi_event_info);
    gtk_container_add(GTK_CONTAINER(frame1), midi_event_info);
    gtk_label_set_justify(GTK_LABEL(midi_event_info), GTK_JUSTIFY_LEFT);

    bind_button = gtk_button_new_with_mnemonic("Bind");
    gtk_widget_show(bind_button);
    gtk_box_pack_start(GTK_BOX(vbox1), bind_button, FALSE, FALSE, 0);

    GtkWidget* unbind_button = gtk_button_new_with_mnemonic("Remove Binding");
    gtk_widget_show(unbind_button);
    gtk_box_pack_start(GTK_BOX(vbox1), unbind_button, FALSE, FALSE, 0);

    GtkWidget* close_button = gtk_button_new_with_mnemonic("Close");
    gtk_widget_show(close_button);
    gtk_box_pack_start(GTK_BOX(vbox1), close_button, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(bind_button), "clicked", (GCallback)bind_clicked, (void*)this);
    g_signal_connect(G_OBJECT(unbind_button), "clicked", (GCallback)unbind_clicked, (void*)this);
    g_signal_connect(G_OBJECT(close_button), "clicked", (GCallback)close_clicked, (void*)this);
    g_signal_connect(G_OBJECT(window), "destroy", (GCallback)close_clicked, (void*)this);

    timer_tag = g_timeout_add(100, (GSourceFunc)timer, (void*)this);

    gtk_widget_show_all(GTK_WIDGET(window));
}

void tX_midiin::midi_binding_gui::window_closed(GtkWidget* widget, gpointer _this) {
    tX_midiin::midi_binding_gui* this_ = (tX_midiin::midi_binding_gui*)_this;

    delete this_;
}

void tX_midiin::midi_binding_gui::unbind_clicked(GtkButton* button, gpointer _this) {
    tX_midiin::midi_binding_gui* this_ = (tX_midiin::midi_binding_gui*)_this;
    GtkTreeModel* model;
    GtkTreeSelection* selection;
    GtkTreeIter iter;
    char tmpstr[128];
    tX_seqpar* param;

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(this_->parameter_treeview));

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, 2, &param, -1);

        param->bound_midi_event.type = tX_midievent::NONE;
        param->bound_midi_event.number = 0;
        param->bound_midi_event.channel = 0;

        snprintf(tmpstr, sizeof(tmpstr), "Type: %d, Number: %d, Channel: %d",
            param->bound_midi_event.type, param->bound_midi_event.number,
            param->bound_midi_event.channel);

        gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, param->get_name(), 1, tmpstr, 2, param, -1);
    } else {
        tx_note("Please select a parameter to unbind first.", true, GTK_WINDOW(this_->window));
    }
}

void tX_midiin::midi_binding_gui::bind_clicked(GtkButton* button, gpointer _this) {
    tX_midiin::midi_binding_gui* this_ = (tX_midiin::midi_binding_gui*)_this;
    GtkTreeModel* model;
    GtkTreeSelection* selection;
    GtkTreeIter iter;
    char tmpstr[128];
    tX_seqpar* param;

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(this_->parameter_treeview));

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, 2, &param, -1);
        if (this_->last_event.type != tX_midievent::NONE) {
            param->bound_midi_event = this_->last_event;
            snprintf(tmpstr, sizeof(tmpstr), "Type: %d, Number: %d, Channel: %d",
                param->bound_midi_event.type, param->bound_midi_event.number,
                param->bound_midi_event.channel);

            gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, param->get_name(), 1, tmpstr, 2, param, -1);
        } else {
            tx_note("Please send midi event to bind first.", true, GTK_WINDOW(this_->window));
        }
    } else {
        tx_note("Please select a parameter to bind first.", true, GTK_WINDOW(this_->window));
    }
}

void tX_midiin::midi_binding_gui::close_clicked(GtkButton* button, gpointer _this) {
    tX_midiin::midi_binding_gui* this_ = (tX_midiin::midi_binding_gui*)_this;

    gtk_widget_destroy(this_->window);
}

gint tX_midiin::midi_binding_gui::timer(gpointer _this) {
    tX_midiin::midi_binding_gui* this_ = (tX_midiin::midi_binding_gui*)_this;
    tX_midievent tmpevent = this_->midi->get_last_event();

    if (tmpevent.type_matches(this_->last_event))
        return TRUE;

    this_->last_event = tmpevent;
    this_->last_event.clear_non_type();

    snprintf(this_->tempstr, sizeof(this_->tempstr),
        "Type: %d (CC=%d, NOTE=%d)\nNumber: %d\nChannel: %d\n",
        this_->last_event.type, tX_midievent::CC, tX_midievent::NOTE,
        this_->last_event.number,
        this_->last_event.channel);

    gtk_label_set_text(GTK_LABEL(this_->midi_event_info), this_->tempstr);

    return TRUE;
}

tX_midiin::midi_binding_gui::~midi_binding_gui() {
    g_source_remove(timer_tag);
}

void tX_midiin::set_midi_learn_sp(tX_seqpar* sp) {
    char buffer[512];

    if (learn_dialog) {
        gtk_widget_destroy(learn_dialog);
    }

    sp_to_learn = sp;

    if (!sp_to_learn)
        return;

    learn_dialog = create_tX_midilearn();
    tX_set_icon(learn_dialog);
    GtkWidget* label = lookup_widget(learn_dialog, "midilabel");

    sprintf(buffer, "Learning MIDI mapping for <b>%s</b>\nfor turntable <b>%s</b>.\n\nWaiting for MIDI event...", sp->get_name(), sp->get_vtt_name());
    gtk_label_set_markup(GTK_LABEL(label), buffer);
    gtk_widget_show(learn_dialog);

    g_signal_connect(G_OBJECT(lookup_widget(learn_dialog, "cancel")), "clicked", G_CALLBACK(tX_midiin::midi_learn_cancel), this);
    g_signal_connect(G_OBJECT(learn_dialog), "destroy", G_CALLBACK(tX_midiin::midi_learn_destroy), this);
}

void tX_midiin::cancel_midi_learn() {
    sp_to_learn = NULL;
    learn_dialog = NULL;
}

gboolean tX_midiin::midi_learn_cancel(GtkWidget* widget, tX_midiin* midi) {
    midi->sp_to_learn = NULL;
    gtk_widget_destroy(midi->learn_dialog);

    return FALSE;
}

gboolean tX_midiin::midi_learn_destroy(GtkWidget* widget, tX_midiin* midi) {
    midi->cancel_midi_learn();

    return FALSE;
}

void tX_midiin::store_connections(FILE* rc, char* indent) {
    gzFile rz = NULL;

    tX_store("%s<midi_connections>\n", indent);

    if (ALSASeqHandle != NULL) {
        strcat(indent, "\t");
        snd_seq_addr_t my_addr;
        my_addr.client = snd_seq_client_id(ALSASeqHandle);
        my_addr.port = portid;

        snd_seq_query_subscribe_t* subs;
        snd_seq_query_subscribe_alloca(&subs);
        snd_seq_query_subscribe_set_root(subs, &my_addr);
        snd_seq_query_subscribe_set_type(subs, SND_SEQ_QUERY_SUBS_WRITE);
        snd_seq_query_subscribe_set_index(subs, 0);

        while (snd_seq_query_port_subscribers(ALSASeqHandle, subs) >= 0) {
            const snd_seq_addr_t* addr;
            addr = snd_seq_query_subscribe_get_addr(subs);

            tX_store("%s<link client=\"%i\" port=\"%i\"/>\n", indent, addr->client, addr->port);
            snd_seq_query_subscribe_set_index(subs, snd_seq_query_subscribe_get_index(subs) + 1);
        }

        indent[strlen(indent) - 1] = 0;
    }

    tX_store("%s</midi_connections>\n", indent);
}

void tX_midiin::restore_connections(xmlNodePtr node) {
    snd_seq_addr_t my_addr;

    if (ALSASeqHandle != NULL) {
        my_addr.client = snd_seq_client_id(ALSASeqHandle);
        my_addr.port = portid;

        if (xmlStrcmp(node->name, (xmlChar*)"midi_connections") == 0) {
            for (xmlNodePtr cur = node->xmlChildrenNode; cur != NULL; cur = cur->next) {
                if (cur->type == XML_ELEMENT_NODE) {
                    if (xmlStrcmp(cur->name, (xmlChar*)"link") == 0) {
                        char* buffer;
                        int client = -1;
                        int port = -1;

                        buffer = (char*)xmlGetProp(cur, (xmlChar*)"client");
                        if (buffer) {
                            sscanf(buffer, "%i", &client);
                        }

                        buffer = (char*)xmlGetProp(cur, (xmlChar*)"port");
                        if (buffer) {
                            sscanf(buffer, "%i", &port);
                        }

                        if ((port >= 0) && (client >= 0)) {
                            snd_seq_addr_t sender_addr;
                            sender_addr.client = client;
                            sender_addr.port = port;

                            snd_seq_port_subscribe_t* subs;
                            snd_seq_port_subscribe_alloca(&subs);
                            snd_seq_port_subscribe_set_sender(subs, &sender_addr);
                            snd_seq_port_subscribe_set_dest(subs, &my_addr);

                            if (snd_seq_subscribe_port(ALSASeqHandle, subs) < 0) {
                                tX_error("tX_midiin::restore_connections() -> failed to connect to: %d:%d.", port, client);
                            }
                        } else {
                            tX_error("tX_midiin::restore_connections() -> invalid port: %d:%d.", port, client);
                        }

                    } else {
                        tX_error("tX_midiin::restore_connections() -> invalid element: %s.", cur->name);
                    }
                }
            }
        } else {
            tX_error("tX_midiin::restore_connections() -> invalid XML element.");
        }
    } else {
        tX_error("tX_midiin::restore_connections() -> Couldn't get ALSA sequencer handle - snd-seq module not loaded?.");
    }
}

extern "C" {
void tX_midiin_store_connections(FILE* rc, char* indent);
void tX_midiin_restore_connections(xmlNodePtr node);
};

void tX_midiin_store_connections(FILE* rc, char* indent) {
    tX_engine::get_instance()->get_midi()->store_connections(rc, indent);
}

void tX_midiin_restore_connections(xmlNodePtr node) {
    tX_engine::get_instance()->get_midi()->restore_connections(node);
}

static inline void cc_map(tX_seqpar* sp, int channel, int number) {
    if (sp->bound_midi_event.type == tX_midievent::NONE) {
        sp->bound_midi_event.type = tX_midievent::CC;
        sp->bound_midi_event.channel = channel;
        sp->bound_midi_event.number = number;
        sp->reset_upper_midi_bound();
        sp->reset_lower_midi_bound();
    }
}

static inline void cc_note(tX_seqpar* sp, int channel, int number) {
    if (sp->bound_midi_event.type == tX_midievent::NONE) {
        sp->bound_midi_event.type = tX_midievent::NOTE;
        sp->bound_midi_event.channel = channel;
        sp->bound_midi_event.number = number;
        sp->reset_upper_midi_bound();
        sp->reset_lower_midi_bound();
    }
}

void tX_midiin::auto_assign_midi_mappings(GtkWidget* widget, gpointer dummy) {
    std::list<vtt_class*>::iterator vtt;
    int ctr = 0;

    /* Works on my hardware :) */
    cc_map(&sp_main_volume, 0, 28);
    cc_map(&sp_main_volume, 0, 29);

    for (vtt = vtt_class::main_list.begin(); (vtt != vtt_class::main_list.end()) && (ctr < 16); vtt++, ctr++) {
        /* These are pretty standard... */
        cc_map((&(*vtt)->sp_volume), ctr, 07);
        cc_map((&(*vtt)->sp_pan), ctr, 10);
        cc_map((&(*vtt)->sp_lp_freq), ctr, 13);
        cc_map((&(*vtt)->sp_lp_reso), ctr, 12);

        /* These are on "general purpose"... */
        cc_map((&(*vtt)->sp_lp_gain), ctr, 16);
        cc_map((&(*vtt)->sp_speed), ctr, 17);
        cc_map((&(*vtt)->sp_pitch), ctr, 18);
        cc_map((&(*vtt)->sp_sync_cycles), ctr, 19);

        /* Sound Controller 6-10 */
        cc_map((&(*vtt)->sp_ec_length), ctr, 75);
        cc_map((&(*vtt)->sp_ec_feedback), ctr, 76);
        cc_map((&(*vtt)->sp_ec_volume), ctr, 77);
        cc_map((&(*vtt)->sp_ec_pan), ctr, 78);

        /* The toggles mapped to notes... */
        cc_note((&(*vtt)->sp_trigger), 0, 60 + ctr);
        cc_note((&(*vtt)->sp_sync_follower), 1, 60 + ctr);
        cc_note((&(*vtt)->sp_loop), 2, 60 + ctr);
        cc_note((&(*vtt)->sp_lp_enable), 3, 60 + ctr);
        cc_note((&(*vtt)->sp_ec_enable), 4, 60 + ctr);
        cc_note((&(*vtt)->sp_mute), 5, 60 + ctr);
        cc_note((&(*vtt)->sp_spin), 6, 60 + ctr);
    }
}

void tX_midiin::clear_midi_mappings(GtkWidget* widget, gpointer dummy) {
    std::list<tX_seqpar*>::iterator sp;

    if (dummy) {
        GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
            GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
            "Really clear all current MIDI mappings?");

        int res = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        if (res != GTK_RESPONSE_YES) {
            return;
        }
    }

    for (sp = tX_seqpar::all->begin(); sp != tX_seqpar::all->end(); sp++) {
        (*sp)->bound_midi_event.type = tX_midievent::NONE;
        (*sp)->bound_midi_event.channel = 0;
        (*sp)->bound_midi_event.number = 0;
        (*sp)->reset_upper_midi_bound();
        (*sp)->reset_lower_midi_bound();
    }
}
#endif // USE_ALSA_MIDI_IN
