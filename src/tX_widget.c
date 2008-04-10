/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2008  Alexander König
 
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

#include <math.h>

#include <gtk/gtk.h>
#include "tX_widget.h"
#include "tX_types.h"
#include "tX_global.h"
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#include <unistd.h>
#endif

#define TX_DEFAULT_SIZE_X 100
#define TX_DEFAULT_SIZE_Y 11

/* forward declaration */
static void gtk_tx_class_init(GtkTxClass *);
static void gtk_tx_init(GtkTx * tx);
GtkWidget *gtk_tx_new(int16_t * wavdata, int wavsamples);
static void gtk_tx_destroy(GtkObject * object);
void gtk_tx_set_data(GtkTx * tx, int16_t * wavdata, int wavsamples);
static void gtk_tx_realize(GtkWidget * widget);
static void gtk_tx_size_request(GtkWidget * widget, GtkRequisition * requisition);
static void gtk_tx_size_allocate(GtkWidget * widget, GtkAllocation * allocation);
static gint gtk_tx_expose(GtkWidget * widget, GdkEventExpose * event);
static void gtk_tx_update(GtkTx * tx);
static void gtk_tx_prepare(GtkWidget * widget);

/* data */
static GtkWidgetClass *parent_class = NULL;

/* widget "methods" */

GType gtk_tx_get_type() {
	static GType tx_type = 0;

 	if (!tx_type) {
		static const GTypeInfo tx_info = {
			sizeof (GtkTxClass),
			NULL,
			NULL,
			(GClassInitFunc) gtk_tx_class_init, 
			NULL,
			NULL,
			sizeof (GtkTx),
        	0,
			(GInstanceInitFunc) gtk_tx_init,
		};

		tx_type = g_type_register_static(GTK_TYPE_WIDGET, "GtkTx", &tx_info, 0);
    }
	
	return tx_type;
}

static void gtk_tx_class_init(GtkTxClass * gclass) {
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
//	widget_class->button_press_event = gtk_tx_button_press;
//	widget_class->button_release_event = gtk_tx_button_release;
//	widget_class->motion_notify_event = gtk_tx_motion_notify;
}

#define COL_BG_FOCUS     0
#define COL_BG_NO_FOCUS  1
#define COL_FG_FOCUS     2
#define COL_FG_NO_FOCUS  3
#define COL_CURSOR       4
#define COL_CURSOR_MUTE  5

void gtk_tx_update_colors(GtkTx *tx)
{
	int i;
	
	if (tx->colors_allocated) {
		gdk_colormap_free_colors(gtk_widget_get_colormap(GTK_WIDGET(tx)), tx->colors, 6);
	}
	
	gdk_color_parse(globals.wav_display_bg_focus, &tx->colors[COL_BG_FOCUS]);
	gdk_color_parse(globals.wav_display_bg_no_focus, &tx->colors[COL_BG_NO_FOCUS]);
	
	gdk_color_parse(globals.wav_display_fg_focus, &tx->colors[COL_FG_FOCUS]);
	gdk_color_parse(globals.wav_display_fg_no_focus, &tx->colors[COL_FG_NO_FOCUS]);
	
	gdk_color_parse(globals.wav_display_cursor, &tx->colors[COL_CURSOR]);
	gdk_color_parse(globals.wav_display_cursor_mute, &tx->colors[COL_CURSOR_MUTE]);
	
	for (i=0; i<6; i++) {
		gdk_colormap_alloc_color(gtk_widget_get_colormap(GTK_WIDGET(tx)), &tx->colors[i], 0, 1);
	}

	tx->colors_allocated=1;
}


static void gtk_tx_init(GtkTx * tx) {
	GdkColormap *priv;

	tx->disp_data = NULL;
	tx->data = NULL;
	tx->colors_allocated=0;
	tx->samples = 0;
	tx->do_showframe = 0;
#ifdef USE_DISPLAY_NORMALIZE
	tx->max_value=-1;
#endif
	
	priv = gdk_colormap_new(gtk_widget_get_visual(GTK_WIDGET(tx)), 6);
	gtk_widget_set_colormap(GTK_WIDGET(tx), priv);

	memset(tx->colors, 0, sizeof(tx->colors));
	
	gtk_tx_update_colors(tx);
	
	tx->current_fg=&tx->colors[COL_FG_NO_FOCUS];
	tx->current_bg=&tx->colors[COL_BG_NO_FOCUS];
	
	tx->spp=1;
	tx->lastmute=-1;
	tx->zoom=0;
	tx->cursor_pos=0;
	tx->cursor_x_pos=0;
}

GtkWidget *gtk_tx_new(int16_t * wavdata, int wavsamples) {
	GtkTx *tx;

	tx = (GtkTx *) g_object_new(gtk_tx_get_type (), NULL);

	tx->data = wavdata;
	tx->samples = wavsamples;
	tx->zoom=0;
	tx->display_x_offset=0;
	
	return GTK_WIDGET(tx);
}

static void gtk_tx_destroy(GtkObject * object) {
	GtkTx *tx;
	g_return_if_fail(object != NULL);
	g_return_if_fail(GTK_IS_TX(object));

	tx=GTK_TX(object);
	
	if (tx->disp_data) { free(tx->disp_data); tx->disp_data=NULL; }
	
	if (GTK_OBJECT_CLASS(parent_class)->destroy) {
		(*GTK_OBJECT_CLASS(parent_class)->destroy) (object);
	}	
}

#define MAX_ZOOM_WIDTH 500000.0

void gtk_tx_set_data(GtkTx * tx, int16_t * wavdata, int wavsamples) {
	g_return_if_fail(tx != NULL);
	g_return_if_fail(GTK_IS_TX(tx));

	tx->data = wavdata;
	tx->samples = wavsamples;
#ifdef USE_DISPLAY_NORMALIZE	
	tx->max_value=-1;
#endif
	
	gtk_tx_prepare(GTK_WIDGET(tx));
	gtk_tx_update(tx);
}

static void gtk_tx_realize(GtkWidget * widget) {
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
	attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;
	attributes.visual = gtk_widget_get_visual(widget);
	attributes.colormap = gtk_widget_get_colormap(widget);
	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

	widget->window = gdk_window_new(widget->parent->window, &attributes, attributes_mask);
	widget->style = gtk_style_attach(widget->style, widget->window);

	gdk_window_set_user_data(widget->window, widget);

	gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);
}

static void gtk_tx_size_request(GtkWidget * widget, GtkRequisition * requisition) {
	requisition->width = TX_DEFAULT_SIZE_X;
	requisition->height = TX_DEFAULT_SIZE_Y;
}

static void gtk_tx_prepare(GtkWidget * widget) {
	int x, sample;
	int16_t *ptr;
	f_prec value;
	GtkTx *tx;
	int avg_pos;
	
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_TX(widget));

	tx = GTK_TX(widget);
	
	tx->yc = widget->allocation.height / 2;

	if (tx->disp_data) { free(tx->disp_data); tx->disp_data=NULL; }

	if (tx->data) {
		int max_spp=tx->samples/widget->allocation.width;
		int min_spp=tx->samples/MAX_ZOOM_WIDTH;
		gdouble diff;
		
		if (min_spp==0) min_spp=1;
		
		diff=max_spp-min_spp;
		
		tx->spp=min_spp+diff*(1.0-tx->zoom);
		tx->display_width = tx->samples/tx->spp;
		
#ifdef USE_DISPLAY_NORMALIZE	
		tx->max_value=-1;
#endif		
		
	    tx->disp_data = (int16_t *) malloc(tx->display_width * sizeof(int16_t));

	    if (tx->disp_data) {			
#ifdef USE_DISPLAY_NORMALIZE		
			if (tx->max_value==-1) {
				/* We haven't figured a max value yet... */
				//puts("searching max...");
		
				for (x = 0, ptr = tx->disp_data; x < tx->display_width; ptr++, x++) {
					value = 0;
					for (sample = x * tx->spp, avg_pos=1; sample < (x + 1) * tx->spp; sample++) {
						value += (abs(tx->data[sample])-value)/(double) avg_pos++;
					}
					if (value>tx->max_value) tx->max_value=value;
					tx->disp_data[x] = value; 
				}
				for (x = 0; x < tx->display_width; x++) {
					f_prec t=tx->disp_data[x]/(double) tx->max_value;
					tx->disp_data[x]=(int) (t * (f_prec) (tx->yc));
				}
			} else {
#endif				
				//puts("have max...");
				/* We have a max value... */
				for (x = 0, ptr = tx->disp_data; x < tx->display_width; ptr++, x++) {
					f_prec t;
					value = 0;
					for (sample = x * tx->spp, avg_pos=1; sample < (x + 1) * tx->spp; sample++) {
						value += (abs(tx->data[sample])-value)/(double) avg_pos++;
					}
#ifdef USE_DISPLAY_NORMALIZE					
					t=value/(double) tx->max_value;
#else
					t=value/32768.0;
#endif					
					tx->disp_data[x] = (int) (t * (f_prec) (tx->yc)); 
				}
#ifdef USE_DISPLAY_NORMALIZE
			}
#endif			
		}
	} else {
	    tx->disp_data = NULL;
	}
	
	tx->cursor_pos=-1;
	tx->lastmute=-1;
	
	//tX_warning("spp: %i samples: %i width %i x %i", tx->spp, tx->samples, tx->display_width, x);
}

static void gtk_tx_size_allocate(GtkWidget * widget, GtkAllocation * allocation) {
	GtkTx *tx;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_TX(widget));
	g_return_if_fail(allocation != NULL);

	widget->allocation = *allocation;

	gtk_tx_prepare(widget);

	if (GTK_WIDGET_REALIZED(widget)) {
	    tx = GTK_TX(widget);
#ifdef USE_DISPLAY_NORMALIZE		
		tx->max_value=-1;
#endif		
	    gdk_window_move_resize(widget->window, allocation->x, allocation->y, allocation->width, allocation->height);
	}
}

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

static gint gtk_tx_expose(GtkWidget * widget, GdkEventExpose * event) {
	GtkTx *tx;
	gint x;
	GdkRectangle *area;
	
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_TX(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

/*	if (event->count > 0) { 
		fprintf(stderr, "Ignoring %i expose events.\n", event->count);
		return FALSE;
	}*/

	area=&event->area;

	tx = GTK_TX(widget);

	gdk_gc_set_foreground(widget->style->fg_gc[widget->state], tx->current_bg);

	gdk_draw_rectangle(widget->window,
		widget->style->fg_gc[widget->state], 1, 
		area->x, area->y,
		area->width, area->height);

	gdk_gc_set_foreground(widget->style->fg_gc[widget->state], tx->current_fg);

	if (tx->disp_data) {
		int max_x=area->x+area->width;

	    for (x =area->x; x < max_x; x++) {
			gdk_draw_line(widget->window,
				widget->style->fg_gc[widget->state], x,
				tx->yc - tx->disp_data[tx->display_x_offset+x], x,
				tx->yc + tx->disp_data[tx->display_x_offset+x]);
	    }
	} else {
	    gdk_draw_line(widget->window, widget->style->fg_gc[widget->state],
			 0, tx->yc, widget->allocation.width, tx->yc);
	}

	return FALSE;
}

void gtk_tx_set_zoom(GtkTx *tx, f_prec zoom) {
	GtkWidget *wid=GTK_WIDGET(tx);
	
	tx->zoom=zoom;
	gtk_tx_prepare(wid);
	gtk_widget_queue_draw(wid);
}

static void gtk_tx_update(GtkTx * tx) {
	g_return_if_fail(tx != NULL);
	g_return_if_fail(GTK_IS_TX(tx));

	gtk_widget_queue_draw(GTK_WIDGET(tx));
}

void gtk_tx_update_pos_display(GtkTx * tx, int sample, int mute) {
	GtkWidget *widget;
	GdkWindow *window;
	GdkGC *gc;

	int x, y, yc, ymax, tmp;
	int current_pos, current_pos_x, x_offset;
	int force_draw=0;

	/* Don't update if not required */

	//current_x = sample / tx->spp + FR_SIZE;
	current_pos = sample / tx->spp;
	
	if ((current_pos == tx->cursor_pos) && 
		(tx->lastmute == mute)) return;
	tx->lastmute = mute;

	/* speedup + easyness */

	widget = GTK_WIDGET(tx);
	window = widget->window;

	gc = widget->style->fg_gc[widget->state];
	yc = tx->yc;
	ymax = widget->allocation.height-1;

	/* clean up last pos */
	
	x = tx->cursor_x_pos;
	
	if (x >= 0) {
	    gdk_gc_set_foreground(gc, tx->current_bg);
	    gdk_draw_line(window, gc, x, 0, x, ymax);

	    gdk_gc_set_foreground(gc, tx->current_fg);
	    y = tx->disp_data[x+tx->display_x_offset];
	    gdk_draw_line(window, gc, x, yc + y, x, yc - y);
	}
	
	/* compute new position */
	if (tx->zoom==0) {
		current_pos_x=current_pos;
		x_offset=0;		
	} else {		
		tmp=widget->allocation.width/2+1;
		
		if (current_pos>tmp) {
			x_offset=current_pos-tmp;
			
			if (x_offset+widget->allocation.width>=tx->display_width) {
				x_offset=tx->display_width-widget->allocation.width;
			}
			
			current_pos_x=current_pos-x_offset;
		} else {
			x_offset=0;
			current_pos_x=current_pos;
		}
		
		if (x_offset!=tx->display_x_offset) {
			int x_move=tx->display_x_offset-x_offset;
			
			if (abs(x_move)<widget->allocation.width) {
				gdk_window_scroll(window, x_move, 0);
			} else {
				/* we've moved so far that there's nothing to keep from our current display */
				force_draw=1;
			}
		}
	}
	
	/* store current_pos */

	tx->cursor_pos = current_pos;
	tx->cursor_x_pos = current_pos_x;
	tx->display_x_offset = x_offset;
	
	/* not drawing current pos - let expose() take care of this... */

	x = current_pos_x;

	if (mute) gdk_gc_set_foreground(gc, &tx->colors[COL_CURSOR_MUTE]);
	else gdk_gc_set_foreground(gc, &tx->colors[COL_CURSOR]);

	gdk_draw_line(window, gc, x, 0, x, ymax);
	
	if (force_draw) {
		gtk_widget_queue_draw_area(widget, 0, 0, widget->allocation.width, widget->allocation.height);
	}
}

void gtk_tx_cleanup_pos_display(GtkTx * tx) {
	GtkWidget *widget;
	GdkWindow *window;
	GdkGC *gc;
	int ymax, yc;

	widget = GTK_WIDGET(tx);
	window = widget->window;
	gc = widget->style->fg_gc[widget->state];
	yc = tx->yc;
	ymax = widget->allocation.height - 1;

	tx->display_x_offset=0;
	tx->cursor_pos=-1;
	tx->cursor_x_pos=-1;
	tx->do_showframe=0;
	//tx->current_fg=&tx->fg;
	
	gtk_widget_queue_draw(widget);
}

void gtk_tx_show_frame(GtkTx *tx, int show) {
	if (show) {
		tx->current_fg=&tx->colors[COL_FG_FOCUS];
		tx->current_bg=&tx->colors[COL_BG_FOCUS];
	} else {
		tx->current_fg=&tx->colors[COL_FG_NO_FOCUS];
		tx->current_bg=&tx->colors[COL_BG_NO_FOCUS];
	}
	
	gtk_widget_queue_draw(GTK_WIDGET(tx));	
}

f_prec gtk_tx_get_zoom(GtkTx *tx) {
	return tx->zoom;
}
