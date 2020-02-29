/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2020  Alexander König

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

*/

#include "tX_panel.h"
#include "tX_vttfx.h"
#include "tX_vtt.h"
#include "tX_pbutton.h"
#include "tX_mastergui.h"
#include <string.h>
#include <stdio.h>

#define WID_DYN TRUE, TRUE, 0
#define WID_FIX FALSE, FALSE, 0

// workaround GtkEntry not having target set to const for some reason
static gchar entries_type[] = "GTK_LIST_BOX_ROW"; 
static const GtkTargetEntry entries[] = { { entries_type, GTK_TARGET_SAME_APP, 0 } };

static vtt_fx *dragged_effect = NULL;
static GtkWidget *dragged_list_box = NULL;

void panel_begin_drag(GtkWidget* widget, GdkDragContext *context, gpointer data) {
	dragged_effect = (vtt_fx *) data;

	GtkWidget *row = gtk_widget_get_ancestor(widget, GTK_TYPE_LIST_BOX_ROW);
	dragged_list_box = gtk_widget_get_ancestor(row, GTK_TYPE_LIST_BOX);
	GtkAllocation allocation;
	gtk_widget_get_allocation(row, &allocation);
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, allocation.width, allocation.height);
	cairo_t *cr = cairo_create(surface);

	gtk_style_context_add_class(gtk_widget_get_style_context(row), "dragging");
	gtk_widget_draw(row, cr);
	gtk_style_context_remove_class (gtk_widget_get_style_context(row), "dragging");

	int x, y;
	gtk_widget_translate_coordinates(widget, row, 0, 0, &x, &y);
	cairo_surface_set_device_offset(surface, -x, -y);
	gtk_drag_set_icon_surface(context, surface);

	cairo_destroy(cr);
	cairo_surface_destroy(surface);
}

void panel_get_drag_data(GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selection_data, guint info, guint time, gpointer data) {
	gtk_selection_data_set(selection_data, gdk_atom_intern_static_string("GTK_LIST_BOX_ROW"), 32, (const guchar *) &widget, sizeof(gpointer));
}

void panel_receive_drag_data(GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint info, guint32 time, gpointer data) {
	int pos = gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(widget));
	GtkWidget* list_box = gtk_widget_get_ancestor(widget, GTK_TYPE_LIST_BOX);
	GtkWidget* row = GTK_WIDGET((gpointer)* (gpointer*) gtk_selection_data_get_data(selection_data));
	GtkWidget* source = gtk_widget_get_ancestor(row, GTK_TYPE_LIST_BOX_ROW);

	if (list_box == dragged_list_box) {
		if (source != widget) {
			g_object_ref (source);
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent (source)), source);
			gtk_list_box_insert(GTK_LIST_BOX(gtk_widget_get_parent (widget)), source, pos);
			if (dragged_effect) {
				vtt_class *vtt=(vtt_class*) dragged_effect->get_vtt();
				vtt->effect_move(dragged_effect, pos);
			}
			g_object_unref (source);
		}
	} else {
		tx_note("Effects can be reordered within the\nsame <b>FX</b> or <b>Stereo FX</b> queue only.");
	}
	dragged_effect = NULL;
	dragged_list_box = NULL;
}

void tX_panel :: minimize(GtkWidget *w, tX_panel *p) {
	if (!p->client_hidden) {
		gtk_widget_hide(p->minimize_button);
		gtk_widget_show(p->maximize_button);
		gtk_widget_hide(p->clientframe);
		p->client_hidden=1;
	} else {
		gtk_widget_hide(p->maximize_button);
		gtk_widget_show(p->minimize_button);
		gtk_widget_show(p->clientframe);
		p->client_hidden=0;
	}

	if (p->controlbox) {
		gtk_widget_queue_draw(p->controlbox);
	}
}

void tX_panel_make_label_bold(GtkWidget *widget) {
	char label[4096];
	snprintf(label, sizeof(label), "<b>%s</b>", gtk_label_get_text(GTK_LABEL(widget)));
	gtk_label_set_markup(GTK_LABEL (widget), label);
}

void tX_panel_make_tooltip(GtkWidget *widget, vtt_fx* effect) {
	char label[4096];
	snprintf(label, sizeof(label), "%s\n\nDrag this handle to reorder effects queue.", effect->get_info_string());
	gtk_widget_set_tooltip_text(widget, label);
}

tX_panel :: tX_panel (const char *name, GtkWidget* controlbox, GCallback close_callback, vtt_fx *effect) {
	client_hidden=0;
	this->controlbox = controlbox;
	add_drywet_button = NULL;
	remove_drywet_button = NULL;

	list_box_row = gtk_list_box_row_new();

	drag_handle = gtk_event_box_new();

	labelbutton=gtk_label_new(name);
	gtk_widget_set_halign(labelbutton, GTK_ALIGN_START);
	tX_panel_make_label_bold(labelbutton);
	gtk_container_add(GTK_CONTAINER(drag_handle), labelbutton);

	topbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	g_object_set(topbox, "margin-start", 10, "margin-end", 0,  "border-width", 0, NULL);
	gtk_container_add_with_properties(GTK_CONTAINER (topbox), drag_handle, "expand", TRUE, NULL);

	minimize_button=create_top_button(MINIMIZE);
	gtk_container_add(GTK_CONTAINER(topbox), minimize_button);
	maximize_button=create_top_button(MAXIMIZE);
	gtk_container_add(GTK_CONTAINER(topbox), maximize_button);

	if (close_callback) {
		close_button = create_top_button(FX_CLOSE);
		gtk_widget_set_name(close_button, "close");
		g_object_set(close_button, "border-width", 0, NULL);
		gtk_container_add(GTK_CONTAINER(topbox), close_button);
	} else {
		close_button = NULL;
	}

	gtk_container_set_border_width(GTK_CONTAINER(topbox), 0);

	mainbox=gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(list_box_row), mainbox);

	clientbox=gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
	clientframe=gtk_frame_new((char *) NULL);
	gtk_container_set_border_width( GTK_CONTAINER(clientframe), 0);
	gtk_container_add(GTK_CONTAINER(clientframe), clientbox);

	gtk_box_pack_start(GTK_BOX(mainbox), topbox, WID_FIX);
	gtk_box_pack_start(GTK_BOX(mainbox), clientframe, WID_FIX);

	gtk_widget_show(labelbutton);
	gtk_widget_show(minimize_button);
	gtk_widget_show(topbox);
	gtk_widget_show(clientbox);
	gtk_widget_show(clientframe);
	gtk_widget_show(mainbox);
	gtk_widget_show(list_box_row);
	gtk_widget_show(drag_handle);

	if (close_callback) {
		gtk_widget_show(close_button);
		g_signal_connect(G_OBJECT(close_button), "clicked", (GCallback) close_callback, (gpointer) effect);
	}

	if (effect) {
		tX_panel_make_tooltip(drag_handle, effect);

		if (effect->has_drywet_feature() != NOT_DRYWET_CAPABLE) {
			add_drywet_button = create_top_button(ADD_DRYWET);
			remove_drywet_button = create_top_button(REMOVE_DRYWET);
		}

		gtk_drag_source_set(drag_handle, GDK_BUTTON1_MASK, entries, 1, GDK_ACTION_MOVE);
		g_signal_connect(drag_handle, "drag-begin", G_CALLBACK(panel_begin_drag), effect);
		g_signal_connect(drag_handle, "drag-data-get", G_CALLBACK(panel_get_drag_data), NULL);

		gtk_drag_dest_set(list_box_row, GTK_DEST_DEFAULT_ALL, entries, 1, GDK_ACTION_MOVE);
		g_signal_connect(list_box_row, "drag-data-received", G_CALLBACK(panel_receive_drag_data), (gpointer) effect);
	}

	g_signal_connect(G_OBJECT(minimize_button), "clicked", (GCallback) tX_panel::minimize, (void *) this);
	g_signal_connect(G_OBJECT(maximize_button), "clicked", (GCallback) tX_panel::minimize, (void *) this);

}

void tX_panel :: add_client_widget(GtkWidget *w)
{
	gtk_box_pack_start(GTK_BOX(clientbox), w, WID_FIX);
	gtk_widget_show(w);
}


tX_panel :: ~tX_panel()
{
	gtk_widget_destroy(clientbox);
	gtk_widget_destroy(clientframe);
	gtk_widget_destroy(topbox);
	gtk_widget_destroy(mainbox);
}
