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
 
    File: tX_widget.c
 
    Description: This contains the implementation of the tx_widget.
    		 This file is based on the GTK+ widget example from
		 the GTK+ 1.2 tutorial.
*/

#define FR_SIZE 3
#define DBL_FR_SIZE 6

#include <math.h>

#include <gtk/gtkwindow.h>
#include "tX_widget.h"
#include "tX_types.h"
#include <malloc.h>

#ifndef WIN32
#include <unistd.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */

#define TX_DEFAULT_SIZE_X 100
#define TX_DEFAULT_SIZE_Y 30

/* pre dec */
    static void gtk_tx_class_init(GtkTxClass *);
    static void gtk_tx_init(GtkTx * tx);
    GtkWidget *gtk_tx_new(int16_t * wavdata, int wavsamples);
    static void gtk_tx_destroy(GtkObject * object);
    void gtk_tx_set_data(GtkTx * tx, int16_t * wavdata, int wavsamples);
    static void gtk_tx_realize(GtkWidget * widget);
    static void gtk_tx_size_request(GtkWidget * widget,
				    GtkRequisition * requisition);
    static void gtk_tx_size_allocate(GtkWidget * widget,
				     GtkAllocation * allocation);
    static gint gtk_tx_expose(GtkWidget * widget, GdkEventExpose * event);
    static void gtk_tx_update(GtkTx * tx);
    static void gtk_tx_prepare(GtkWidget * widget);

/* Local data */

    static GtkWidgetClass *parent_class = NULL;

     guint gtk_tx_get_type() {
	static guint tx_type = 0;

	if (!tx_type) {
	    GtkTypeInfo tx_info = {
		"GtkTx",
		sizeof(GtkTx),
		sizeof(GtkTxClass),
		(GtkClassInitFunc) gtk_tx_class_init,
		(GtkObjectInitFunc) gtk_tx_init,
		/* reserved */ NULL,
		/* reserved */ NULL,
		/* reserved */ NULL
	    };

	     tx_type = gtk_type_unique(gtk_widget_get_type(), &tx_info);
	}

	return tx_type;
    }

    static void
     gtk_tx_class_init(GtkTxClass * gclass) {
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) gclass;
	widget_class = (GtkWidgetClass *) gclass;

	parent_class = gtk_type_class(gtk_widget_get_type());

	object_class->destroy = gtk_tx_destroy;

	widget_class->realize = gtk_tx_realize;
	widget_class->expose_event = gtk_tx_expose;
	widget_class->size_request = gtk_tx_size_request;
	widget_class->size_allocate = gtk_tx_size_allocate;
//  widget_class->button_press_event = gtk_tx_button_press;
//  widget_class->button_release_event = gtk_tx_button_release;
//  widget_class->motion_notify_event = gtk_tx_motion_notify;
    }

    void gtk_tx_mk_col(GtkTx * tx, GdkColor * col, float r, float g,
		       float b) {
	float max = 65535.0;

	col->red = (gint) (r * max);
	col->green = (gint) (g * max);
	col->blue = (gint) (b * max);
//      printf("r: %8i, g: %8i, b: %8i\n", col->red, col->green, col->blue);
	gdk_colormap_alloc_color(gtk_widget_get_colormap(GTK_WIDGET(tx)),
				 col, 1, 1);
    }

    static void
     gtk_tx_init(GtkTx * tx) {
	GdkColormap *priv;

	tx->disp_data = NULL;

	tx->data = NULL;
	tx->samples = 0;

	tx->do_showframe = 0;

	priv = gdk_colormap_new(gtk_widget_get_visual(GTK_WIDGET(tx)), 6);
	gtk_widget_set_colormap(GTK_WIDGET(tx), priv);

	gtk_tx_mk_col(tx, &tx->bg, 0, 0, 0);

	gtk_tx_mk_col(tx, &tx->fg, 0, 1, 0);

	gtk_tx_mk_col(tx, &tx->busy_fg, 1, 1, 1);
	gtk_tx_mk_col(tx, &tx->busy_bg, 1, 0.4, 0.4);

	gtk_tx_mk_col(tx, &tx->mute_fg, 0, 1, 1);
	gtk_tx_mk_col(tx, &tx->mute_bg, 0, 0, 1);
	gtk_tx_mk_col(tx, &tx->framecol, 1, 0, 0);
    }

    GtkWidget *gtk_tx_new(int16_t * wavdata, int wavsamples) {
	GtkTx *tx;

	tx = gtk_type_new(gtk_tx_get_type());

	tx->data = wavdata;
	tx->samples = wavsamples;

//  gtk_tx_prepare(GTK_WIDGET(tx));     

	return GTK_WIDGET(tx);
    }

    static void
     gtk_tx_destroy(GtkObject * object) {
	g_return_if_fail(object != NULL);
	g_return_if_fail(GTK_IS_TX(object));

	if (GTK_OBJECT_CLASS(parent_class)->destroy)
	    (*GTK_OBJECT_CLASS(parent_class)->destroy) (object);
    }


    void gtk_tx_set_data(GtkTx * tx, int16_t * wavdata, int wavsamples) {
	g_return_if_fail(tx != NULL);
	g_return_if_fail(GTK_IS_TX(tx));

	tx->data = wavdata;
	tx->samples = wavsamples;

	gtk_tx_prepare(GTK_WIDGET(tx));
	gtk_tx_update(tx);
    }

    static void
     gtk_tx_realize(GtkWidget * widget) {
	GtkTx *tx;
	GdkWindowAttr attributes;
	gint attributes_mask;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_TX(widget));

	GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);
	tx = GTK_TX(widget);

	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.event_mask = gtk_widget_get_events(widget) |
	    GDK_EXPOSURE_MASK;
	attributes.visual = gtk_widget_get_visual(widget);
	attributes.colormap = gtk_widget_get_colormap(widget);

	attributes_mask =
	    GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

	widget->window =
	    gdk_window_new(widget->parent->window, &attributes,
			   attributes_mask);

	widget->style = gtk_style_attach(widget->style, widget->window);

	gdk_window_set_user_data(widget->window, widget);

	gtk_style_set_background(widget->style, widget->window,
				 GTK_STATE_NORMAL);
    }

    static void
     gtk_tx_size_request(GtkWidget * widget, GtkRequisition * requisition) {
	requisition->width = TX_DEFAULT_SIZE_X;
	requisition->height = TX_DEFAULT_SIZE_Y;
    }

    static void
     gtk_tx_prepare(GtkWidget * widget) {
	int x, sample;
	f_prec temp;
	int16_t *ptr;
	int16_t value;

	GtkTx *tx;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_TX(widget));

	tx = GTK_TX(widget);

	tx->spp = tx->samples / (widget->allocation.width - DBL_FR_SIZE);
	tx->yc = widget->allocation.height / 2;

	if (tx->disp_data)
	    free(tx->disp_data);

	if (tx->data) {

	    tx->disp_data =
		(int16_t *) malloc((widget->allocation.width - DBL_FR_SIZE)
				   * sizeof(int16_t));

	    if (tx->disp_data)
		for (x = 0, ptr = tx->disp_data;
		     x < widget->allocation.width - DBL_FR_SIZE;
		     ptr++, x++) {
		    value = tx->data[x * tx->spp];
		    for (sample = x * tx->spp; sample < (x + 1) * tx->spp;
			 sample++) {
			value = (value + tx->data[sample]) / 2;
		    }
		    temp = ((f_prec) value) / 32767.0;
		    tx->disp_data[x] =
			(int) (temp * (f_prec) (tx->yc - FR_SIZE));
		}

	} else
	    tx->disp_data = NULL;
    }

    static void
     gtk_tx_size_allocate(GtkWidget * widget, GtkAllocation * allocation) {
	GtkTx *tx;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_TX(widget));
	g_return_if_fail(allocation != NULL);

	widget->allocation = *allocation;

	gtk_tx_prepare(widget);

	if (GTK_WIDGET_REALIZED(widget)) {
	    tx = GTK_TX(widget);

	    gdk_window_move_resize(widget->window,
				   allocation->x, allocation->y,
				   allocation->width, allocation->height);

	}
    }

    static gint gtk_tx_expose(GtkWidget * widget, GdkEventExpose * event) {
	GtkTx *tx;
	gint x, pos;

	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_TX(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	if (event->count > 0)
	    return FALSE;

	tx = GTK_TX(widget);

	gdk_gc_set_foreground(widget->style->fg_gc[widget->state],
			      &tx->bg);

	gdk_draw_rectangle(widget->window,
			   widget->style->fg_gc[widget->state], 1, 0, 0,
			   widget->allocation.width,
			   widget->allocation.height);

	gdk_gc_set_foreground(widget->style->fg_gc[widget->state],
			      &tx->fg);

	if (tx->disp_data) {
	    for (x = FR_SIZE, pos = 0;
		 x < widget->allocation.width - FR_SIZE; x++, pos++) {
		gdk_draw_line(widget->window,
			      widget->style->fg_gc[widget->state], x,
			      tx->yc - tx->disp_data[pos], x,
			      tx->yc + tx->disp_data[pos]);
	    }
	} else {
	    gdk_draw_line(widget->window,
			  widget->style->fg_gc[widget->state],
			  FR_SIZE,
			  tx->yc,
			  widget->allocation.width - FR_SIZE, tx->yc);
	}

	gtk_tx_show_frame(tx, tx->do_showframe);
	return FALSE;
    }

    static void
     gtk_tx_update(GtkTx * tx) {
	g_return_if_fail(tx != NULL);
	g_return_if_fail(GTK_IS_TX(tx));

	gtk_widget_draw(GTK_WIDGET(tx), NULL);
    }

    void
     gtk_tx_prepare_pos_display(GtkTx * tx) {
	tx->lastpos = -1;
    }

    void
     gtk_tx_update_pos_display(GtkTx * tx, int sample, int mute) {
	GtkWidget *widget;
	GdkWindow *window;
	GdkGC *gc;

	int current_x, x, y, yc, ymax;

	/* Don't update if not required */

	current_x = sample / tx->spp + FR_SIZE;

	if ((current_x == tx->lastpos) && (tx->lastmute == mute))
	    return;
	tx->lastmute = mute;

	/* speedup + easyness */

	widget = GTK_WIDGET(tx);
	window = widget->window;

	if (current_x > widget->allocation.width - FR_SIZE - 2)
	    return;

	gc = widget->style->fg_gc[widget->state];
	yc = tx->yc;
	ymax = widget->allocation.height - FR_SIZE - 1;

	/* Clean up last pos */

	x = tx->lastpos;

	if (x >= FR_SIZE) {
	    gdk_gc_set_foreground(gc, &tx->bg);
	    gdk_draw_line(window, gc, x, FR_SIZE, x, ymax);

	    gdk_gc_set_foreground(gc, &tx->fg);
	    y = tx->disp_data[x - FR_SIZE];
	    gdk_draw_line(window, gc, x, yc + y, x, yc - y);
	}
	/* store current_pos */

	tx->lastpos = current_x;

	/* draw current_pos */

	x = current_x;

	if (mute)
	    gdk_gc_set_foreground(gc, &tx->mute_bg);
	else
	    gdk_gc_set_foreground(gc, &tx->busy_bg);

	gdk_draw_line(window, gc, x, FR_SIZE, x, ymax);
    }

    void gtk_tx_cleanup_pos_display(GtkTx * tx) {
	GtkWidget *widget;
	GdkWindow *window;
	GdkGC *gc;
	int x, y, ymax, yc;

	widget = GTK_WIDGET(tx);
	window = widget->window;
	gc = widget->style->fg_gc[widget->state];
	yc = tx->yc;
	ymax = widget->allocation.height - FR_SIZE - 1;

	x = tx->lastpos;

	if (x >= FR_SIZE) {
	    gdk_gc_set_foreground(gc, &tx->bg);
	    gdk_draw_line(window, gc, x, FR_SIZE, x, ymax);

	    gdk_gc_set_foreground(gc, &tx->fg);
	    y = tx->disp_data[x - FR_SIZE];
	    gdk_draw_line(window, gc, x, yc + y, x, yc - y);
	}
    }

    void gtk_tx_show_frame(GtkTx * tx, int show) {
	GtkWidget *widget;
	GdkWindow *window;
	GdkGC *gc;
	int i;

	widget = GTK_WIDGET(tx);
	window = widget->window;
	gc = widget->style->fg_gc[widget->state];


	tx->do_showframe = show;

	if (show) {
	    gdk_gc_set_foreground(gc, &tx->framecol);
	} else {
	    gdk_gc_set_foreground(gc, &tx->bg);
	}

	for (i = 0; i < FR_SIZE; i++) {
	    gdk_draw_rectangle(window, gc, 0, i, i,
			       widget->allocation.width - (2 * i + 1),
			       widget->allocation.height - (2 * i + 1));
	}
    }

#ifdef __cplusplus
}
#endif				/* __cplusplus */
