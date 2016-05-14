/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2016  Alexander König
 
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
static void gtk_tx_destroy(GtkWidget * widget);
void gtk_tx_set_data(GtkTx * tx, int16_t * wavdata, int wavsamples);
static void gtk_tx_realize(GtkWidget * widget);

static void gtk_tx_get_preferred_width (GtkWidget *widget, gint *minimal_height, gint *natural_height);
static void gtk_tx_get_preferred_height (GtkWidget *widget, gint *minimal_height, gint *natural_height);

static void gtk_tx_size_allocate(GtkWidget * widget, GtkAllocation * allocation);
static gboolean gtk_tx_draw(GtkWidget * widget, cairo_t *cr);
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
	GtkWidgetClass *widget_class;
	
	widget_class = (GtkWidgetClass *) gclass;
	
	parent_class = (GtkWidgetClass *) g_type_class_peek(gtk_widget_get_type());
	
	widget_class->destroy = gtk_tx_destroy;
	
	widget_class->realize = gtk_tx_realize;
	widget_class->draw = gtk_tx_draw;
	widget_class->get_preferred_height = gtk_tx_get_preferred_height;
	widget_class->get_preferred_width = gtk_tx_get_preferred_width;
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
	gdk_rgba_parse(&tx->colors[COL_BG_FOCUS], globals.wav_display_bg_focus);
	tx->colors[COL_BG_FOCUS].alpha=1.0;
	gdk_rgba_parse(&tx->colors[COL_BG_NO_FOCUS], globals.wav_display_bg_no_focus);
	tx->colors[COL_BG_NO_FOCUS].alpha=1.0;
	
	gdk_rgba_parse(&tx->colors[COL_FG_FOCUS], globals.wav_display_fg_focus);
	tx->colors[COL_FG_FOCUS].alpha=1.0;
	gdk_rgba_parse(&tx->colors[COL_FG_NO_FOCUS], globals.wav_display_fg_no_focus);
	tx->colors[COL_FG_NO_FOCUS].alpha=1.0;
	
	gdk_rgba_parse(&tx->colors[COL_CURSOR], globals.wav_display_cursor);
	tx->colors[COL_CURSOR].alpha=1.0;
	gdk_rgba_parse(&tx->colors[COL_CURSOR_MUTE], globals.wav_display_cursor_mute);
	tx->colors[COL_CURSOR_MUTE].alpha=1.0;
}


static void gtk_tx_init(GtkTx * tx) {
	tx->disp_data = NULL;
	tx->data = NULL;
	tx->samples = 0;
	tx->do_showframe = 0;
#ifdef USE_DISPLAY_NORMALIZE
	tx->max_value=-1;
#endif
	
	memset(tx->colors, 0, sizeof(tx->colors));
	
	gtk_tx_update_colors(tx);
	
	tx->current_fg=tx->audio_colors_focus;
	tx->current_bg=&tx->colors[COL_BG_NO_FOCUS];
	
	tx->audio_colors_focus = NULL;
	tx->audio_colors_nofocus = NULL;
	
	tx->spp=1;
	tx->lastmute=-1;
	tx->zoom=0;
	tx->cursor_pos=0;
	tx->cursor_x_pos=0;
	
	tx->surface = NULL;
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

static void gtk_tx_destroy(GtkWidget * widget) {
	GtkTx *tx;
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_TX(widget));

	tx=GTK_TX(widget);
	
	if (tx->disp_data) { free(tx->disp_data); tx->disp_data=NULL; }
	
	if (GTK_WIDGET_CLASS(parent_class)->destroy) {
		(*GTK_WIDGET_CLASS(parent_class)->destroy) (widget);
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
	GdkWindowAttr attributes;
	gint attributes_mask;
	GtkTx *tx;
	
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_TX(widget));

	tx = GTK_TX(widget);
	gtk_widget_set_realized(widget, TRUE);

	GtkAllocation allocation;
	gtk_widget_get_allocation(widget, &allocation);
	attributes.x = allocation.x;
	attributes.y = allocation.y;
	attributes.width = allocation.width;
	attributes.height = allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;
	attributes.visual = gtk_widget_get_visual(widget);
	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;

	gtk_widget_set_window(widget, gdk_window_new(gtk_widget_get_parent_window(widget), &attributes, attributes_mask));

	gdk_window_set_user_data(gtk_widget_get_window(widget), widget);

	if (tx->surface) {
		cairo_surface_destroy (tx->surface);
	}
	
	tx->surface = gdk_window_create_similar_surface (gtk_widget_get_window(widget), CAIRO_CONTENT_COLOR, allocation.width, allocation.height);
}

static void gtk_tx_get_preferred_width (GtkWidget *widget, gint *minimal_width, gint *natural_width)
{
  *minimal_width = *natural_width = TX_DEFAULT_SIZE_X;
}

static void gtk_tx_get_preferred_height (GtkWidget *widget, gint *minimal_height, gint *natural_height)
{
    *minimal_height = *natural_height = TX_DEFAULT_SIZE_Y;
}

static void gtk_tx_prepare(GtkWidget * widget) {
	int x, sample;
	int16_t *ptr;
	f_prec value;
	GtkTx *tx;
	int avg_pos, color;
	gboolean *success;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_TX(widget));

	tx = GTK_TX(widget);
	
	GtkAllocation allocation;
	GdkRGBA midColor;
	gboolean fg = (tx->current_fg == tx->audio_colors_focus);
	
	if (tx->audio_colors_focus) { 
		free(tx->audio_colors_focus); 
		free(tx->audio_colors_nofocus); 
		
		tx->audio_colors_focus = NULL; 
		tx->audio_colors_nofocus = NULL; 
	} else {
		fg = FALSE;
	}
	
	// update tx->yc
	
	gtk_widget_get_allocation(widget, &allocation);
	tx->yc = allocation.height / 2;
	
	// allocate colors
	
	tx->audio_colors_focus = (GdkRGBA *) malloc(tx->yc * sizeof(GdkRGBA));
	tx->audio_colors_nofocus = (GdkRGBA *) malloc(tx->yc * sizeof(GdkRGBA));
	
	success = (gboolean *) malloc(tx->yc * sizeof(gboolean));

	// no focus colors

	midColor.red = tx->colors[COL_BG_NO_FOCUS].red + (tx->colors[COL_FG_NO_FOCUS].red - tx->colors[COL_BG_NO_FOCUS].red) / 4;
	midColor.green = tx->colors[COL_BG_NO_FOCUS].green + (tx->colors[COL_FG_NO_FOCUS].green - tx->colors[COL_BG_NO_FOCUS].green) / 4;
	midColor.blue = tx->colors[COL_BG_NO_FOCUS].blue + (tx->colors[COL_FG_NO_FOCUS].blue - tx->colors[COL_BG_NO_FOCUS].blue) / 4;
	
	for (color=0 ; color < tx->yc; color++) {
		float dist = (float) color / (float) tx->yc;
		
		tx->audio_colors_nofocus[color].red = midColor.red + dist*(tx->colors[COL_FG_NO_FOCUS].red - midColor.red);
		tx->audio_colors_nofocus[color].green = midColor.green + dist*(tx->colors[COL_FG_NO_FOCUS].green - midColor.green);
		tx->audio_colors_nofocus[color].blue = midColor.blue + dist*(tx->colors[COL_FG_NO_FOCUS].blue - midColor.blue);
		tx->audio_colors_nofocus[color].alpha = 1.0;
	}
	// focus colors

	midColor.red = tx->colors[COL_BG_FOCUS].red + (tx->colors[COL_FG_FOCUS].red - tx->colors[COL_BG_FOCUS].red) / 4;
	midColor.green = tx->colors[COL_BG_FOCUS].green + (tx->colors[COL_FG_FOCUS].green - tx->colors[COL_BG_FOCUS].green) / 4;
	midColor.blue = tx->colors[COL_BG_FOCUS].blue + (tx->colors[COL_FG_FOCUS].blue - tx->colors[COL_BG_FOCUS].blue) / 4;
	
	for (color=0 ; color < tx->yc; color++) {
		float dist = (float) color / (float) tx->yc;
		
		tx->audio_colors_focus[color].red = midColor.red + dist*(tx->colors[COL_FG_FOCUS].red - midColor.red);
		tx->audio_colors_focus[color].green = midColor.green + dist*(tx->colors[COL_FG_FOCUS].green - midColor.green);
		tx->audio_colors_focus[color].blue = midColor.blue + dist*(tx->colors[COL_FG_FOCUS].blue - midColor.blue);
		tx->audio_colors_focus[color].alpha = 1.0;
	}
	
	if (fg) {
		tx->current_fg = tx->audio_colors_focus;
	} else {
		tx->current_fg = tx->audio_colors_nofocus;
	}
	
	// give up success
	free(success);

	if (tx->disp_data) { free(tx->disp_data); tx->disp_data=NULL; }

	if (tx->data) {
		int max_spp=tx->samples/allocation.width;
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
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_TX(widget));
	g_return_if_fail(allocation != NULL);

	gtk_widget_set_allocation(widget, allocation);

	gtk_tx_prepare(widget);

	if (gtk_widget_get_realized(widget)) {
#ifdef USE_DISPLAY_NORMALIZE		
	    GtkTx *tx = GTK_TX(widget);
		tx->max_value=-1;
#endif		
	    gdk_window_move_resize(gtk_widget_get_window(widget), allocation->x, allocation->y, allocation->width, allocation->height);
	}
}

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

static gboolean gtk_tx_draw(GtkWidget * widget, cairo_t *cr) {
	GtkTx *tx;
	gint x;
	GdkRectangle area;
	
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_TX(widget), FALSE);

	gdk_cairo_get_clip_rectangle(cr, &area);
	
	tx = GTK_TX(widget);
	
	cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
	cairo_set_source_surface (cr, tx->surface, 0, 0);
	cairo_set_line_width(cr,1);

	gdk_cairo_set_source_rgba (cr, tx->current_bg);

	cairo_rectangle(cr, area.x, area.y, area.width, area.height);
	cairo_fill(cr);

	if (tx->disp_data) {
		int max_x=area.x+area.width+1;

	    for (x =area.x; x < max_x; x++) {
			int dy = tx->disp_data[tx->display_x_offset+x];
			gdk_cairo_set_source_rgba (cr, &tx->current_fg[dy]);
			cairo_move_to (cr, x, tx->yc - dy);
			cairo_line_to (cr, x, tx->yc + dy+1);
			cairo_stroke (cr);
	    }
	} else {
		GtkAllocation allocation;
		gtk_widget_get_allocation(widget, &allocation);
		
		gdk_cairo_set_source_rgba (cr, tx->current_fg);
		cairo_move_to (cr, 0, tx->yc);
		cairo_line_to (cr, allocation.width, tx->yc);
		cairo_stroke (cr);
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
	cairo_t *cr;
	
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
	window = gtk_widget_get_window(widget);

	yc = tx->yc;
	GtkAllocation allocation;
	gtk_widget_get_allocation(widget, &allocation);
	ymax = allocation.height-1;

	/* clean up last pos */
	
	x = tx->cursor_x_pos;
	
	cr = gdk_cairo_create (gtk_widget_get_window(widget));
	cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
	cairo_set_source_surface (cr, tx->surface, 0, 0);
	cairo_set_line_width(cr,1);
	
	if (x >= 0) {
		gdk_cairo_set_source_rgba (cr, tx->current_bg);
		
		cairo_move_to (cr, x, 0);
		cairo_line_to (cr, x, ymax);
		cairo_stroke (cr);
		
	    y = tx->disp_data[x+tx->display_x_offset];
	    gdk_cairo_set_source_rgba (cr, &tx->current_fg[y]);
	    
		cairo_move_to (cr, x, yc + y);
		cairo_line_to (cr, x, yc - y+1);
		cairo_stroke (cr);
	}
	
	/* compute new position */
	if (tx->zoom==0) {
		current_pos_x=current_pos;
		x_offset=0;		
	} else {		
		tmp=allocation.width/2+1;
		
		if (current_pos>tmp) {
			x_offset=current_pos-tmp;
			
			if (x_offset+allocation.width>=tx->display_width) {
				x_offset=tx->display_width-allocation.width;
			}
			
			current_pos_x=current_pos-x_offset;
		} else {
			x_offset=0;
			current_pos_x=current_pos;
		}
		
		if (x_offset!=tx->display_x_offset) {
			int x_move=tx->display_x_offset-x_offset;
			
			if (abs(x_move)<allocation.width) {
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

	if (mute) gdk_cairo_set_source_rgba(cr, &tx->colors[COL_CURSOR_MUTE]);
	else gdk_cairo_set_source_rgba(cr, &tx->colors[COL_CURSOR]);

	cairo_move_to (cr, x, 0);
	cairo_line_to (cr, x, ymax);
	cairo_stroke (cr);
	
	cairo_destroy (cr);
	
	if (force_draw) {
		gtk_widget_queue_draw_area(widget, 0, 0, allocation.width, allocation.height);
	}
}

void gtk_tx_cleanup_pos_display(GtkTx * tx) {
	GtkWidget *widget;
	GtkAllocation allocation;

	widget = GTK_WIDGET(tx);
	gtk_widget_get_allocation(widget, &allocation);

	tx->display_x_offset=0;
	tx->cursor_pos=-1;
	tx->cursor_x_pos=-1;
	tx->do_showframe=0;
	//tx->current_fg=&tx->fg;
	
	gtk_widget_queue_draw(widget);
}

void gtk_tx_show_frame(GtkTx *tx, int show) {
	if (show) {
		tx->current_fg=tx->audio_colors_focus;
		tx->current_bg=&tx->colors[COL_BG_FOCUS];
	} else {
		tx->current_fg=tx->audio_colors_nofocus;
		tx->current_bg=&tx->colors[COL_BG_NO_FOCUS];
	}
	
	gtk_widget_queue_draw(GTK_WIDGET(tx));	
}

f_prec gtk_tx_get_zoom(GtkTx *tx) {
	return tx->zoom;
}
