/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2022  Alexander König

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

    File: tX_loaddlg.cc

    Description: Displays the progress indicator dialog for file
                 loading.
*/
#include "tX_loaddlg.h"
#include "tX_maingui.h"
#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

GtkWidget* ld_loaddlg = (GtkWidget*)NULL;
GtkWidget* ld_single_l = (GtkWidget*)NULL;
GtkWidget* ld_single_p = (GtkWidget*)NULL;
GtkWidget* ld_multi_l = (GtkWidget*)NULL;
GtkWidget* ld_multi_p = (GtkWidget*)NULL;
// GtkWindow *ld_window=(GtkWindow *)NULL;

int ld_mode;
int ld_count;
int ld_current;

gfloat ld_old_prog;

#define WID_DYN TRUE, TRUE, 0
#define WID_FIX FALSE, FALSE, 0
#define add_widget_dyn(wid)                          \
    ;                                                \
    gtk_box_pack_start(GTK_BOX(vbox), wid, WID_DYN); \
    gtk_widget_show(wid);

#define add_widget_fix(wid)                          \
    ;                                                \
    gtk_box_pack_start(GTK_BOX(vbox), wid, WID_FIX); \
    gtk_widget_show(wid);

#define gtk_flush()                                       \
    ;                                                     \
    {                                                     \
        int ctr = 0;                                      \
        while (gtk_events_pending()) {                    \
            ctr++;                                        \
            if (ctr > 5)                                  \
                break;                                    \
            gtk_main_iteration();                         \
            gdk_display_flush(gdk_display_get_default()); \
        }                                                 \
    }

int ld_create_loaddlg(int mode, int count) {
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget* dummy;

    if (ld_loaddlg)
        return 1;

    ld_mode = mode;
    ld_count = count;

    ld_loaddlg = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(ld_loaddlg), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_title(GTK_WINDOW(ld_loaddlg), "terminatorX - loading");
    gtk_window_set_transient_for(GTK_WINDOW(ld_loaddlg), GTK_WINDOW(main_window));

    gtk_container_set_border_width(GTK_CONTAINER(ld_loaddlg), 5);
    gtk_container_add(GTK_CONTAINER(ld_loaddlg), vbox);
    gtk_widget_set_size_request(vbox, 400, -1);
    gtk_widget_show(vbox);

    if (mode == TX_LOADDLG_MODE_MULTI) {
        ld_multi_l = gtk_label_new("Loading Set");
        add_widget_fix(ld_multi_l);

        ld_multi_p = gtk_progress_bar_new();
        add_widget_fix(ld_multi_p);

        dummy = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
        add_widget_fix(dummy);

        ld_current = 0;
    }

    ld_single_l = gtk_label_new("Loading File");
    add_widget_fix(ld_single_l);

    ld_single_p = gtk_progress_bar_new();
    add_widget_fix(ld_single_p);

    gtk_window_set_modal(GTK_WINDOW(ld_loaddlg), TRUE);
    gtk_widget_realize(ld_loaddlg);
    gdk_window_set_decorations(gtk_widget_get_window(ld_loaddlg), (GdkWMDecoration)0);
    gtk_widget_show(ld_loaddlg);
    gdk_window_set_cursor(gtk_widget_get_window(ld_loaddlg), tX_cursor::get_cursor());

    gtk_flush();

    return 0;
}

char* strip_path(char* name) {
    char* tmp;

    if (!name)
        return NULL;

    tmp = strrchr(name, (int)'/');

    if (tmp) {
        if (strlen(tmp) > 1) {
            tmp++;
        }
    } else
        tmp = name;

    return tmp;
}

void ld_set_setname(char* name) {
    char* setname;
    char buffer[1024];

    setname = strip_path(name);
    sprintf(buffer, "Loading tX-set [%s]", setname);
    gtk_label_set_text(GTK_LABEL(ld_multi_l), buffer);
    gtk_flush();
}

void ld_set_filename(char* name) {
    char* filename;
    char buffer[1024];
    gfloat setprog;

    ld_current++;
    ld_old_prog = -1;
    filename = strip_path(name);
    if (ld_mode == TX_LOADDLG_MODE_MULTI) {
        sprintf(buffer, "Loading file No. %i of %i [%s]", ld_current, ld_count, filename);
    } else {
        sprintf(buffer, "Loading file [%s]", filename);
    }
    gtk_label_set_text(GTK_LABEL(ld_single_l), buffer);

    if (ld_mode == TX_LOADDLG_MODE_MULTI) {
        setprog = (((float)ld_current) / ((float)ld_count));
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ld_multi_p), setprog);
        gtk_flush();
    }
    gtk_flush();
}

void ld_set_progress(gfloat progress) {
    progress = floor(progress * 200.0) / 200.0;
    if (progress > 1.0)
        progress = 1.0;

    if (progress != ld_old_prog) {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ld_single_p), progress);
        gtk_flush();
    }

    ld_old_prog = progress;
}

void ld_destroy() {
    if (ld_loaddlg) {
        gtk_widget_hide(ld_loaddlg);
        gtk_widget_destroy(ld_loaddlg);
    }

    ld_loaddlg = NULL;
    mg_update_status();
}
