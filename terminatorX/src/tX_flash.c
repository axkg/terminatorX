/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2006  Alexander K�nig
 
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
 
    File: tX_flash.c
 
    Description: This contains the implementation of the tx_flash_flash widget.
*/    

#include <math.h>

#include <gtk/gtkwindow.h>
#define IS_TX_FLASH 1
#include "tX_flash.h"
#include "tX_types.h"
#include "tX_global.h"
#include <malloc.h>

#ifndef WIN32
#include <unistd.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MAX_VALUE 1.0f
// 32767
#define RED_BORDER 0.8f
// 25000
#define TX_FLASH_DEFAULT_SIZE_X 25
#define TX_FLASH_DEFAULT_SIZE_Y 30
#define DY 5
#define DX 5
#define DMINIX 2
#define S_MINIX 2
#define L_MINIX 3
#define DLEVEL 3
#define MAX_MAX_CYCLES 30;

/* pre dec */
static void gtk_tx_flash_class_init (GtkTxFlashClass *);
static void gtk_tx_flash_init (GtkTxFlash *tx_flash);
GtkWidget* gtk_tx_flash_new ();
static void gtk_tx_flash_destroy (GtkObject *object);
static void gtk_tx_flash_realize (GtkWidget *widget);
static void gtk_tx_flash_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void gtk_tx_flash_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static gint gtk_tx_flash_expose (GtkWidget *widget, GdkEventExpose *event);

/* Local data */

static GtkWidgetClass *parent_class = NULL;

GType gtk_tx_flash_get_type ()
{
	static GType tx_flash_type = 0;

 	if (!tx_flash_type) {
		static const GTypeInfo tx_flash_info = {
			sizeof (GtkTxFlashClass),
			NULL,
			NULL,
			(GClassInitFunc) gtk_tx_flash_class_init, 
			NULL,
			NULL,
			sizeof (GtkTxFlash),
        	0,
			(GInstanceInitFunc) gtk_tx_flash_init,
		};

		tx_flash_type = g_type_register_static(GTK_TYPE_WIDGET, "GtkTxFlash", &tx_flash_info, 0);
    }
	
	return tx_flash_type;
}

static void gtk_tx_flash_class_init (GtkTxFlashClass *gclass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	
	object_class = (GtkObjectClass*) gclass;
	widget_class = (GtkWidgetClass*) gclass;
	
	parent_class = gtk_type_class (gtk_widget_get_type ());
	
	object_class->destroy = gtk_tx_flash_destroy;
	
	widget_class->realize = gtk_tx_flash_realize;
	widget_class->expose_event = gtk_tx_flash_expose;
	widget_class->size_request = gtk_tx_flash_size_request;
	widget_class->size_allocate = gtk_tx_flash_size_allocate;
	//widget_class->button_press_event = gtk_tx_flash_button_press;
	//widget_class->button_release_event = gtk_tx_flash_button_release;
	//widget_class->motion_notify_event = gtk_tx_flash_motion_notify;
}

enum {
	COL_BG,
	COL_NORM,
	COL_NORM_HALF,
	COL_LOUD,
	COL_LOUD_HALF,
	NO_COLS
};

/* c=a+(a-b)*x; */

inline void mk_half(double s, GdkColor *a, GdkColor *b, GdkColor *c)
{
	c->red=a->red-(a->red-b->red)*s;
	c->green=a->green-(a->green-b->green)*s;
	c->blue=a->blue-(a->blue-b->blue)*s;
}

void gtk_tx_flash_update_colors(GtkTxFlash *tx)
{
	int i;
	
	if (tx->colors_allocated) {
		gdk_colormap_free_colors(gtk_widget_get_colormap(GTK_WIDGET(tx)), tx->colors, NO_COLS);
	}
	
	gdk_color_parse(globals.vu_meter_bg, &tx->colors[COL_BG]);
	gdk_color_parse(globals.vu_meter_loud, &tx->colors[COL_LOUD]);
	gdk_color_parse(globals.vu_meter_normal, &tx->colors[COL_NORM]);	
	mk_half(globals.vu_meter_border_intensity, &tx->colors[COL_BG], &tx->colors[COL_LOUD], &tx->colors[COL_LOUD_HALF]);
	mk_half(globals.vu_meter_border_intensity, &tx->colors[COL_BG], &tx->colors[COL_NORM], &tx->colors[COL_NORM_HALF]);

	for (i=0; i<NO_COLS; i++) {
		gdk_colormap_alloc_color(gtk_widget_get_colormap(GTK_WIDGET(tx)), &tx->colors[i], 0, 1);
	}

	tx->colors_allocated=1;
}

static void gtk_tx_flash_init (GtkTxFlash *tx_flash)
{
	GdkColormap *priv;
	
	tx_flash->colors_allocated=0;
	priv=gdk_colormap_new(gtk_widget_get_visual(GTK_WIDGET(tx_flash)), 6);

	gtk_widget_set_colormap(GTK_WIDGET(tx_flash), priv);
	gtk_tx_flash_update_colors(tx_flash);
}

GtkWidget* gtk_tx_flash_new ()
{
	GtkTxFlash *tx_flash;
	tx_flash = (GtkTxFlash *)g_object_new(gtk_tx_flash_get_type (), NULL);	
	
	// tX_msg("creating a new flash: %08x\n", tx_flash);
	return GTK_WIDGET (tx_flash);
}

static void gtk_tx_flash_destroy (GtkObject *object)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_IS_TX_FLASH (object));
	
	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(*GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void gtk_tx_flash_realize (GtkWidget *widget)
{
	GtkTxFlash *tx_flash;
	GdkWindowAttr attributes;
	gint attributes_mask;
	
	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_TX_FLASH (widget));
	
	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
	tx_flash = GTK_TX_FLASH (widget);
	
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.event_mask = gtk_widget_get_events (widget) | 
		GDK_EXPOSURE_MASK;
	attributes.visual = gtk_widget_get_visual (widget);
	attributes.colormap = gtk_widget_get_colormap (widget);
	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
	
	widget->window = gdk_window_new (widget->parent->window, &attributes, attributes_mask);
	widget->style = gtk_style_attach (widget->style, widget->window);
	
	gdk_window_set_user_data (widget->window, widget);
	gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
}

static void gtk_tx_flash_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
	requisition->width = TX_FLASH_DEFAULT_SIZE_X;
	requisition->height = TX_FLASH_DEFAULT_SIZE_Y;
}

static void gtk_tx_flash_prepare(GtkWidget *widget)
{
	GtkTxFlash *tx_flash;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_TX_FLASH (widget));
	
	tx_flash=GTK_TX_FLASH(widget);

	tx_flash->levels=(widget->allocation.height-(2*DY))/DLEVEL;
	tx_flash->channel[0].last_level=0;
	tx_flash->channel[1].last_level=0;
	tx_flash->channel[0].max=0;
	tx_flash->channel[1].max=0;
	tx_flash->level_value=MAX_VALUE/(f_prec) tx_flash->levels;
	tx_flash->red_level=(RED_BORDER/tx_flash->level_value);
	
	tx_flash->channel[0].x1=DMINIX+S_MINIX+2;
	tx_flash->channel[1].x2=widget->allocation.width-tx_flash->channel[0].x1-1;
	
	if (widget->allocation.width%2>0) {
		// odd
		tx_flash->center_expand=0;
		tx_flash->channel[0].x2=widget->allocation.width/2-2;
	} else {
		// even
		tx_flash->center_expand=1;
		tx_flash->channel[0].x2=widget->allocation.width/2-3;
	}
	tx_flash->channel[1].x1=widget->allocation.width/2+2;
	
	//tX_msg("flash: width %i: left %i, right %i", widget->allocation.width, tx_flash->channel[0].x2-tx_flash->channel[0].x1, tx_flash->channel[1].x2-tx_flash->channel[1].x1);
}

static void gtk_tx_flash_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
	GtkTxFlash *tx_flash;
	
	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_TX_FLASH (widget));
	g_return_if_fail (allocation != NULL);
	
	widget->allocation = *allocation;
	
	gtk_tx_flash_prepare(widget);
	
	if (GTK_WIDGET_REALIZED (widget)) {
		tx_flash = GTK_TX_FLASH (widget);
		
		gdk_window_move_resize (widget->window,
				  allocation->x, allocation->y,
				  allocation->width, allocation->height);
	}
}

static void gtk_tx_flash_paint_yourself(GtkWidget *widget)
{
	GtkTxFlash *tx_flash;
	gint i, x11, x12,x21, x22, y, middle;
	int long_level;
	
	tx_flash = GTK_TX_FLASH (widget);
	
	gdk_gc_set_foreground(widget->style->fg_gc[widget->state], &tx_flash->colors[COL_BG]);
	
	gdk_draw_rectangle(widget->window, widget->style->fg_gc[widget->state], 1, 0, 0, widget->allocation.width,widget->allocation.height); 
	
	gdk_gc_set_foreground(widget->style->fg_gc[widget->state], &tx_flash->colors[COL_NORM_HALF]);
	
	x12=DMINIX+S_MINIX;
	x21=widget->allocation.width-1-x12;
	middle=widget->allocation.width/2;
	
	for (i=0, y=widget->allocation.height-DY; i<=tx_flash->levels; y-=DLEVEL, i++) {
		if (i==0) {
			long_level=1;
		} else if (i==tx_flash->red_level-1) {
			long_level=1;
		} else if (i==tx_flash->red_level) {
			long_level=1;
			gdk_gc_set_foreground(widget->style->fg_gc[widget->state], &tx_flash->colors[COL_LOUD_HALF]);
		} else if (i==tx_flash->levels) {
			long_level=1;
		} else long_level=0;
		
		if (long_level) {
			x11=x12-L_MINIX;
			x22=x21+L_MINIX;
		} else {
			x11=x12-S_MINIX;
			x22=x21+S_MINIX;		
		}
		
		gdk_draw_line(widget->window, widget->style->fg_gc[widget->state], x11, y, x12, y);
		gdk_draw_line(widget->window, widget->style->fg_gc[widget->state], x21, y, x22, y);
		
		if (tx_flash->center_expand) {
			gdk_draw_line(widget->window, widget->style->fg_gc[widget->state], middle-1, y, middle, y);			
		} else {
			gdk_draw_point(widget->window, widget->style->fg_gc[widget->state], middle, y);
		}
	}
}

static gint gtk_tx_flash_expose (GtkWidget *widget, GdkEventExpose *event)
{
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_TX_FLASH (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);
	
	if (event->count > 0)
		return FALSE;
	
	gtk_tx_flash_prepare(widget);
	gtk_tx_flash_paint_yourself(widget);  
	
	return FALSE;
} 


static void gtk_tx_flash_set_channel_level(GtkTxFlash *tx_flash, f_prec new_value, struct flash_channel *channel);

void gtk_tx_flash_set_level(GtkWidget *widget, f_prec left_channel, f_prec right_channel)
{
	GtkTxFlash *tx_flash;
	
	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_TX_FLASH (widget));
	
	tx_flash = GTK_TX_FLASH (widget);
	
	gtk_tx_flash_set_channel_level(tx_flash, left_channel, &tx_flash->channel[0]);
	gtk_tx_flash_set_channel_level(tx_flash, right_channel, &tx_flash->channel[1]);
}

static void gtk_tx_flash_set_channel_level(GtkTxFlash *tx_flash, f_prec new_value, struct flash_channel *channel)
{
	GtkWidget *widget=GTK_WIDGET(tx_flash);
	gint i, y;
	int new_level;
	int red=0;
	
	new_level=(int) (new_value/tx_flash->level_value);
	
	// tX_msg("setting level: %5d for widget %08x channel %08x\n", new_level, tx_flash, channel);
	
	if (new_level>tx_flash->levels) 
		new_level=tx_flash->levels;
	
	if (new_level>channel->max) {
		channel->max=new_level;
		tx_flash->max_cycles=MAX_MAX_CYCLES;
	} else {
		tx_flash->max_cycles--;
	}
	
	if (tx_flash->max_cycles <= 0) {
		y=widget->allocation.height-(DY+(channel->max)*DLEVEL);
		gdk_gc_set_foreground(widget->style->fg_gc[widget->state], &tx_flash->colors[COL_BG]);
		gdk_draw_line(widget->window, widget->style->fg_gc[widget->state], channel->x1, y, channel->x2, y);
		
		if (channel->max>0) {
			channel->max--;
			y+=DLEVEL;
			if (channel->max>tx_flash->red_level) {
				gdk_gc_set_foreground(widget->style->fg_gc[widget->state], &tx_flash->colors[COL_LOUD]);
			} else {
				gdk_gc_set_foreground(widget->style->fg_gc[widget->state], &tx_flash->colors[COL_NORM]);
			}
			
			gdk_draw_line(widget->window, widget->style->fg_gc[widget->state], channel->x1, y, channel->x2, y);
		}
	}
	
	if (new_level==channel->last_level) 
		return;
	
	if (new_level<channel->last_level) {
		new_level=channel->last_level*globals.flash_response;
	}
	
	if (new_level>channel->last_level) {
		gdk_gc_set_foreground(widget->style->fg_gc[widget->state], &tx_flash->colors[COL_NORM]);
		
		for (i=channel->last_level, y=widget->allocation.height-(DY+channel->last_level*DLEVEL); i<=new_level; y-=DLEVEL, i++) {
			if (!red) {
				if (i>=tx_flash->red_level) {
					gdk_gc_set_foreground(widget->style->fg_gc[widget->state], &tx_flash->colors[COL_LOUD]);
					red=1;
				}
			}
			gdk_draw_line(widget->window, widget->style->fg_gc[widget->state], channel->x1, y, channel->x2, y);
		}
	} else {
		gdk_gc_set_foreground(widget->style->fg_gc[widget->state], &tx_flash->colors[COL_BG]);
		
		if (channel->last_level==channel->max) {
			i=channel->last_level-1;
		} else {
			i=channel->last_level;
		}
		
		for (y=widget->allocation.height-(DY+i*DLEVEL); i>new_level; y+=DLEVEL, i--) {
			gdk_draw_line(widget->window, widget->style->fg_gc[widget->state], channel->x1, y, channel->x2, y);
		}
	}
	channel->last_level=new_level;
}

void
gtk_tx_flash_clear (GtkWidget *widget)
{
	GtkTxFlash *tx_flash;

	tx_flash = GTK_TX_FLASH (widget);
	
	tx_flash->channel[0].max=0;
	tx_flash->channel[1].max=0;
	tx_flash->channel[0].last_level=0;
	tx_flash->channel[1].last_level=0;
	tx_flash->max_cycles=0;
	
	gtk_tx_flash_paint_yourself(widget);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
