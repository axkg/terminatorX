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
static void gtk_tx_prepare(GtkWidget * widget);
static void gtk_tx_update_render_buffer(GtkTx* tx);

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
}

#define COL_FOCUS 0 
#define COL_NO_FOCUS 1

#define COL_BG_FOCUS     0
#define COL_BG_NO_FOCUS  1
#define COL_FG_FOCUS     2
#define COL_FG_NO_FOCUS  3
#define COL_CURSOR       4
#define COL_CURSOR_MUTE  5

void gtk_tx_update_colors(GtkTx *tx) {
	int step;

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

	for (step = 0; step < GTK_TX_HISTORY_LENGTH; step++) {
		double frac = (1.0 / ((double) GTK_TX_HISTORY_LENGTH + 2.0)) * ((double) step + 1.0);

		GdkRGBA *color = &tx->history_colors[step];
		color->red = tx->colors[COL_CURSOR].red;
		color->green = tx->colors[COL_CURSOR].green;
		color->blue = tx->colors[COL_CURSOR].blue;
		color->alpha = frac*frac/2;
	}
}


static void gtk_tx_init(GtkTx * tx) {
	tx->disp_data = NULL;
	tx->data = NULL;
	tx->samples = 0;
#ifdef USE_DISPLAY_NORMALIZE
	tx->max_value=-1;
#endif
	
	memset(tx->colors, 0, sizeof(tx->colors));
	memset(tx->history_colors, 0, sizeof(tx->history_colors));

	gtk_tx_update_colors(tx);
	
	tx->current_fg=tx->audio_colors_focus;
	tx->current_bg=&tx->colors[COL_BG_NO_FOCUS];
	
	tx->audio_colors_focus = NULL;
	tx->audio_colors_nofocus = NULL;
	
	tx->spp=1;
	tx->zoom=0;
	tx->cursor_pos=0;
	tx->cursor_x_pos=0;
	
	tx->render_buffer_surface_a = NULL;
	tx->render_buffer_surface_b = NULL;
	tx->current_render_buffer_surface = NULL;
	tx->previous_render_buffer_surface = NULL;

	tx->render_buffer_x_offset = -1;
	tx->render_buffer_display_width = -1;
	tx->render_buffer_fg = NULL;
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
	
	if (tx->render_buffer_surface_a) {
		cairo_surface_destroy(tx->render_buffer_surface_a);
		cairo_surface_destroy(tx->render_buffer_surface_b);
		tx->render_buffer_surface_a = NULL; 
		tx->render_buffer_surface_b = NULL;
	}

	if (tx->disp_data) { 
		free(tx->disp_data);
		tx->disp_data=NULL;
	}
	
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
	gtk_widget_queue_draw(GTK_WIDGET(tx));
}

static void gtk_tx_realize(GtkWidget * widget) {
	GdkWindowAttr attributes;
	gint attributes_mask;
	
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_TX(widget));

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
}

static void gtk_tx_get_preferred_width (GtkWidget *widget, gint *minimal_width, gint *natural_width) {
  *minimal_width = *natural_width = TX_DEFAULT_SIZE_X;
}

static void gtk_tx_get_preferred_height (GtkWidget *widget, gint *minimal_height, gint *natural_height) {
	*minimal_height = *natural_height = TX_DEFAULT_SIZE_Y;
}

static void gtk_tx_reallocate_disp_data(GtkWidget *widget) {
	GtkAllocation allocation;
	GtkTx *tx = GTK_TX(widget);
	int x, sample, avg_pos;
	int16_t *ptr;
	f_prec value;

	gtk_widget_get_allocation(widget, &allocation);

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
	
}

static void gtk_tx_prepare(GtkWidget * widget) {
	GtkTx *tx;
	int color;

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
	tx->xc = allocation.width / 2;
	tx->xmax = allocation.width;
	tx->yc = allocation.height / 2;
	tx->ymax = allocation.height;

	// allocate colors
	
	tx->audio_colors_focus = (GdkRGBA *) malloc(tx->yc * sizeof(GdkRGBA));
	tx->audio_colors_nofocus = (GdkRGBA *) malloc(tx->yc * sizeof(GdkRGBA));
	
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
	
	gtk_tx_reallocate_disp_data(widget);
}

static void gtk_tx_size_allocate(GtkWidget * widget, GtkAllocation * allocation) {
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_TX(widget));
	g_return_if_fail(allocation != NULL);

	GtkTx *tx = GTK_TX(widget);

	gtk_widget_set_allocation(widget, allocation);
	
	if (tx->render_buffer_surface_a) {
		cairo_surface_destroy(tx->render_buffer_surface_a);
		cairo_surface_destroy(tx->render_buffer_surface_b);
		tx->render_buffer_surface_a = NULL;
		tx->render_buffer_surface_b = NULL;
	}
	
	tx->render_buffer_x_offset = -1;
	tx->render_buffer_display_width = -1;
	tx->render_buffer_fg = NULL;

	if (gtk_widget_get_window(widget) != NULL) {
		tx->render_buffer_surface_a = gdk_window_create_similar_surface(gtk_widget_get_window(widget), CAIRO_CONTENT_COLOR, allocation->width, allocation->height);
		tx->render_buffer_surface_b = gdk_window_create_similar_surface(gtk_widget_get_window(widget), CAIRO_CONTENT_COLOR, allocation->width, allocation->height);
	}
	
	tx->current_render_buffer_surface = tx->render_buffer_surface_a;
	tx->previous_render_buffer_surface = tx->render_buffer_surface_b;

	gtk_tx_prepare(widget);

	if (gtk_widget_get_realized(widget)) {
#ifdef USE_DISPLAY_NORMALIZE		
		GtkTx *tx = GTK_TX(widget);
		tx->max_value=-1;
#endif		
	    gdk_window_move_resize(gtk_widget_get_window(widget), allocation->x, allocation->y, allocation->width, allocation->height);
	}
}

void gtk_tx_set_zoom(GtkTx *tx, f_prec zoom, int is_playing) {
	GtkWidget *wid=GTK_WIDGET(tx);
	
	if (zoom != tx->zoom) {
		tx->zoom=zoom;
		gtk_tx_reallocate_disp_data(wid);
		if (!is_playing || (zoom < 0.01)) {
			gtk_widget_queue_draw(wid);
		}
	}
}

#define draw_line(x1, y1, x2, y2, rgba) { gdk_cairo_set_source_rgba(cr, rgba); cairo_move_to(cr, x1, y1); cairo_line_to(cr, x2, y2); cairo_stroke(cr); }
#define draw_line2(x1, y1, x2, y2) { cairo_move_to(cr, x1, y1); cairo_line_to(cr, x2, y2); cairo_stroke(cr); }
#define draw_sample(x, y1, y2, rgba) draw_line(x, y1, x, y2, rgba)
#define draw_sample2(x, y1, y2) draw_line2(x, y1, x, y2)
#define draw_rectangle(rect, rgba) { gdk_cairo_set_source_rgba(cr, rgba); cairo_rectangle(cr, rect.x, rect.y, rect.width, rect.height); cairo_fill(cr); }
#define draw_rectangle2(x, y, width, height, rgba) { gdk_cairo_set_source_rgba(cr, rgba); cairo_rectangle(cr, x, y, width, height); cairo_fill(cr); }

static void gtk_tx_update_render_buffer(GtkTx *tx) {
	int x, src_x;
	
	if ((tx->render_buffer_display_width != tx->display_width) || 
			(tx->render_buffer_fg != tx->current_fg)) {
		// need to redraw all samples
		cairo_t *cr = cairo_create(tx->render_buffer_surface_a);
		tx->current_render_buffer_surface = tx->render_buffer_surface_a;
		tx->previous_render_buffer_surface = tx->render_buffer_surface_b;

		draw_rectangle2(0, 0, tx->xmax, tx->ymax, tx->current_bg);
		for (x=0, src_x=tx->display_x_offset; x < tx->xmax; x++, src_x++) {
			int dy = tx->disp_data[src_x];
			draw_sample(x, tx->yc-dy, tx->yc+dy+1, &tx->current_fg[dy]);
		}
		cairo_destroy(cr);

		tx->render_buffer_display_width = tx->display_width;
		tx->render_buffer_x_offset = tx->display_x_offset;
		tx->render_buffer_fg = tx->current_fg;
	} else if (tx->render_buffer_x_offset != tx->display_x_offset) {
		// switch buffers
		cairo_surface_t *surface = tx->current_render_buffer_surface;
		tx->current_render_buffer_surface = tx->previous_render_buffer_surface;
		tx->previous_render_buffer_surface = surface;
		int motion = tx->render_buffer_x_offset - tx->display_x_offset;
		cairo_t *cr = cairo_create(tx->current_render_buffer_surface);

		int cur_x, start_x, stop_x;
		int width = tx->xmax - abs(motion);

		if (motion > 0) {
			// move right
			cur_x = motion;
			start_x = 0;
			stop_x = motion;
		} else {
			// move left
			cur_x = 0;
			start_x = width;
			stop_x = tx->xmax;
		}
    
		cairo_set_source_surface(cr, tx->previous_render_buffer_surface, motion, 0);
		cairo_rectangle(cr, cur_x, 0, width, tx->ymax);
		cairo_fill(cr);

		// draw the rest;
		draw_rectangle2(start_x, 0, stop_x - start_x, tx->ymax, tx->current_bg);
		for (x=start_x, src_x=tx->display_x_offset+start_x; x < stop_x; x++, src_x++) {
			int dy = tx->disp_data[src_x];
			draw_sample(x, tx->yc-dy, tx->yc+dy+1, &tx->current_fg[dy]);
		}

		cairo_destroy(cr);

		tx->render_buffer_x_offset = tx->display_x_offset;
	}
}

static gboolean gtk_tx_draw(GtkWidget * widget, cairo_t *cr) {
	GtkTx *tx;
	gint x;
	GdkRectangle area;
	
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_TX(widget), FALSE);

	tx = GTK_TX(widget);
	
	gdk_cairo_get_clip_rectangle(cr, &area);
	
	cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
	cairo_set_line_width(cr, 1);

	if (tx->disp_data) {
		int step;
		int x_offset;

		if (tx->zoom > 0.0) {
		    x_offset= tx->cursor_pos > tx->xc ? tx->cursor_pos-tx->xc : 0;
			if (x_offset+tx->xmax > tx->display_width) {
				x_offset = tx->display_width-tx->xmax;
			}
		} else {
		    x_offset = 0;		    	
		}

		tx->cursor_x_pos = tx->cursor_pos-x_offset;
		tx->display_x_offset = x_offset;

		gtk_tx_update_render_buffer(tx);
		cairo_set_source_surface(cr, tx->current_render_buffer_surface, 0, 0);
		cairo_paint(cr);

		tx->cursor_history[tx->cursor_history_offset] = tx->cursor_pos;
		tx->cursor_history_offset++;
		if (tx->cursor_history_offset >= GTK_TX_HISTORY_LENGTH) {
			tx->cursor_history_offset -= GTK_TX_HISTORY_LENGTH;
		}

		if (globals.wav_display_history && !tx->mute) {
			int prev_sample_pos = -1;
			/* draw history */

			int max_dist = 1;
			for (step = 0; step < GTK_TX_HISTORY_LENGTH; step++) {
			    max_dist = tX_max(max_dist, abs(tx->cursor_pos - tx->cursor_history[step]));
			}			

			for (step = 0; step < GTK_TX_HISTORY_LENGTH; step++) {
				int history_pos = tx->cursor_history_offset - step;

				if (history_pos < 0) {
					history_pos += GTK_TX_HISTORY_LENGTH;
				}

				int sample_pos = tx->cursor_history[history_pos];

				if ((sample_pos >= 0) && (prev_sample_pos >= 0)) {
					int sample_x_pos = sample_pos - tx->display_x_offset;
					int prev_sample_x_pos = prev_sample_pos - tx->display_x_offset;
					int min, max;

					gdk_cairo_set_source_rgba(cr, &tx->history_colors[GTK_TX_HISTORY_LENGTH-step]);
					if (prev_sample_x_pos > sample_x_pos) {
						min = sample_x_pos;
						max = prev_sample_x_pos;
					} else {
						min = prev_sample_x_pos;
						max = sample_x_pos;
					}
					
					if ((max - min) <= tx->xc) {
						for (x=min; x < max; x++) {
							double scale = (1.0-(fabs(x-tx->cursor_x_pos) / (double) max_dist));
							int dist = scale * scale * 0.5 * (double) tx->ymax;
							int value = tx->disp_data[x+tx->display_x_offset] + dist;
							draw_sample2(x, tx->yc-value, tx->yc+value+1);
						}
					} else {
						for (x = 0; x < min; x++) {
							double scale = (1.0-(fabs(x-tx->cursor_x_pos) / (double) max_dist));
							int dist = scale * scale * 0.5 * (double) tx->ymax;
							int value = tx->disp_data[x+tx->display_x_offset] + dist;
							draw_sample2(x, tx->yc-value, tx->yc+value+1);
						}
						for (x= max; x < tx->xmax; x++) {
							double scale = (1.0-(fabs(x-tx->cursor_x_pos) / (double) max_dist));
							int dist = scale * scale * 0.5 * (double) tx->ymax;
							int value = tx->disp_data[x+tx->display_x_offset] + dist;
							draw_sample2(x, tx->yc-value, tx->yc+value+1);
						}
					}
				}

				prev_sample_pos = sample_pos;
			}
		}
		/* draw cursor */
		draw_sample(tx->cursor_x_pos, 0, tx->ymax, tx->mute ? &tx->colors[COL_CURSOR_MUTE] : &tx->colors[COL_CURSOR]);
	} else {
		draw_rectangle(area, tx->current_bg);
		draw_line(area.x, tx->yc, area.width, tx->yc, tx->current_fg);
	}
	
	return FALSE;
}

void gtk_tx_update_pos_display(GtkTx * tx, int sample, int mute) {
	GtkWidget *widget = GTK_WIDGET(tx);
	
	tx->cursor_pos = sample / tx->spp;
	tx->mute = mute;

	gtk_widget_queue_draw(widget);
}

void gtk_tx_cleanup_pos_display(GtkTx * tx) {
	GtkWidget *widget;
	GtkAllocation allocation;
	int step;

	widget = GTK_WIDGET(tx);
	gtk_widget_get_allocation(widget, &allocation);

	tx->display_x_offset=0;
	tx->cursor_pos=-1;
	tx->cursor_x_pos=-1;

	for (step = 0; step < GTK_TX_HISTORY_LENGTH; step++) {
		tx->cursor_history[step] = -1;
	}
	
	gtk_widget_queue_draw(widget);
}

void gtk_tx_show_focus(GtkTx *tx, int show) {
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
