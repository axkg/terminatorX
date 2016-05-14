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
 
    File: tX_dial.ch
 
    Description: Implements the dial widget - this widget is based on the 
    gtk_dial example from the gtk+ tutorial which is
    Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
*/    

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "tX_knobloader.h"

#include "tX_dial.h"

#define SCROLL_DELAY_LENGTH  300
#define TX_DIAL_DEFAULT_SIZE 100

/* Forward declarations */

static void gtk_tx_dial_class_init		(GtkTxDialClass *klass);
static void gtk_tx_dial_init			(GtkTxDial *tx_dial);
static void gtk_tx_dial_destroy			(GtkWidget *widget);
static void gtk_tx_dial_realize			(GtkWidget *widget);

static void gtk_tx_dial_get_preferred_width (GtkWidget *widget, gint *minimal_height, gint *natural_height);
static void gtk_tx_dial_get_preferred_height (GtkWidget *widget, gint *minimal_height, gint *natural_height);
static void gtk_tx_dial_size_allocate	(GtkWidget *widget, GtkAllocation *allocation);
static gboolean gtk_tx_dial_draw		(GtkWidget *widget, cairo_t* cairo);
static gint gtk_tx_dial_button_press	(GtkWidget *widget, GdkEventButton *event);
static gint gtk_tx_dial_button_release	(GtkWidget *widget, GdkEventButton *event);
static gint gtk_tx_dial_motion_notify	(GtkWidget *widget, GdkEventMotion *event);
static void gtk_tx_dial_update_mouse	(GtkTxDial *tx_dial, gint x, gint y);
static void gtk_tx_dial_update			(GtkTxDial *tx_dial);
static void gtk_tx_dial_adjustment_changed			(GtkAdjustment *adjustment, gpointer data);
static void gtk_tx_dial_adjustment_value_changed 	(GtkAdjustment *adjustment, gpointer data);

/* Local data */

static GtkWidgetClass *parent_class = NULL;

#define calc_image(f,i); i=(gint) ((f - tx_dial->old_lower)/(tx_dial->old_range) * ((float) TX_MAX_KNOB_PIX)); if(i>TX_MAX_KNOB_PIX) i=TX_MAX_KNOB_PIX; else if (i<0) i=0;

GType gtk_tx_dial_get_type ()
{
	static GType tx_dial_type = 0;

 	if (!tx_dial_type) {
		static const GTypeInfo tx_dial_info = {
			sizeof (GtkTxDialClass),
			NULL,
			NULL,
			(GClassInitFunc) gtk_tx_dial_class_init, 
			NULL,
			NULL,
			sizeof (GtkTxDial),
        	0,
			(GInstanceInitFunc) gtk_tx_dial_init,
		};

		tx_dial_type = g_type_register_static(GTK_TYPE_WIDGET, "GtkTxDial", &tx_dial_info, 0);
    }
	
	return tx_dial_type;
}

static void gtk_tx_dial_class_init (GtkTxDialClass *class)
{
	GtkWidgetClass *widget_class;

	widget_class = (GtkWidgetClass*) class;

	parent_class = (GtkWidgetClass*) g_type_class_peek (gtk_widget_get_type ());

	widget_class->destroy = gtk_tx_dial_destroy;

	widget_class->realize = gtk_tx_dial_realize;
	widget_class->draw = gtk_tx_dial_draw;
	widget_class->get_preferred_height = gtk_tx_dial_get_preferred_height;
	widget_class->get_preferred_width = gtk_tx_dial_get_preferred_width;
	widget_class->size_allocate = gtk_tx_dial_size_allocate;
	widget_class->button_press_event = gtk_tx_dial_button_press;
	widget_class->button_release_event = gtk_tx_dial_button_release;
	widget_class->motion_notify_event = gtk_tx_dial_motion_notify;
}

static void gtk_tx_dial_init (GtkTxDial *tx_dial)
{
	tx_dial->button = 0;

	tx_dial->old_value = 0.0;
	tx_dial->old_lower = 0.0;
	tx_dial->old_upper = 0.0;
	tx_dial->old_range = 0.0; // Dangerous!

	tx_dial->old_image = 0;

	tx_dial->yofs=0;
	tx_dial->xofs=0;

	tx_dial->adjustment = NULL;
}

GtkWidget* gtk_tx_dial_new (GtkAdjustment *adjustment)
{
	GtkTxDial *tx_dial;
	
	tx_dial = (GtkTxDial *) g_object_new(gtk_tx_dial_get_type(), NULL);
	
	if (!adjustment) {
		adjustment = (GtkAdjustment*) gtk_adjustment_new (0.0, 0.0, 0.0,
					  0.0, 0.0, 0.0);
	}

	gtk_tx_dial_set_adjustment (tx_dial, adjustment);
	g_object_ref (G_OBJECT (tx_dial->adjustment));

	return GTK_WIDGET (tx_dial);
}

static void gtk_tx_dial_destroy (GtkWidget *widget)
{
	GtkTxDial *tx_dial;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_TX_DIAL (widget));
	
	tx_dial = GTK_TX_DIAL (widget);

	if (tx_dial->adjustment)
		g_object_unref (G_OBJECT (tx_dial->adjustment));
	
	if (GTK_WIDGET_CLASS (parent_class)->destroy)
		(*GTK_WIDGET_CLASS (parent_class)->destroy) (widget);
}

GtkAdjustment* gtk_tx_dial_get_adjustment (GtkTxDial *tx_dial)
{
	g_return_val_if_fail (tx_dial != NULL, NULL);
	g_return_val_if_fail (GTK_IS_TX_DIAL (tx_dial), NULL);
	
	return tx_dial->adjustment;
}

void gtk_tx_dial_set_adjustment (GtkTxDial *tx_dial, GtkAdjustment *adjustment)
{
	g_return_if_fail (tx_dial != NULL);
	g_return_if_fail (GTK_IS_TX_DIAL (tx_dial));
	
	if (tx_dial->adjustment) {
		g_signal_handlers_disconnect_matched(G_OBJECT(tx_dial->adjustment),
			G_SIGNAL_MATCH_DATA, 0, 0, 0, 0,
			(gpointer) tx_dial);
		g_object_unref (G_OBJECT (tx_dial->adjustment));
	}
	
	tx_dial->adjustment = adjustment;
	g_object_ref (G_OBJECT (tx_dial->adjustment));

	g_signal_connect (G_OBJECT (adjustment), "changed",
			  (GCallback) gtk_tx_dial_adjustment_changed,
			  (gpointer) tx_dial);
	g_signal_connect (G_OBJECT (adjustment), "value_changed",
			  (GCallback) gtk_tx_dial_adjustment_value_changed,
			  (gpointer) tx_dial);

	tx_dial->old_value = gtk_adjustment_get_value(adjustment);
	tx_dial->old_lower = gtk_adjustment_get_lower(adjustment);
	tx_dial->old_upper = gtk_adjustment_get_upper(adjustment);
	tx_dial->old_range = gtk_adjustment_get_upper(adjustment) - gtk_adjustment_get_lower(adjustment);

	calc_image(gtk_adjustment_get_value(adjustment),tx_dial->old_image);

	gtk_tx_dial_update (tx_dial);
}

static void gtk_tx_dial_realize (GtkWidget *widget)
{
	GdkWindowAttr attributes;
	gint attributes_mask;
	
	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_TX_DIAL (widget));
	
	gtk_widget_set_realized(widget, TRUE);
	
	GtkAllocation allocation;
	gtk_widget_get_allocation(widget, &allocation);
	attributes.x = allocation.x;
	attributes.y = allocation.y;
	attributes.width = allocation.width;
	attributes.height = allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.event_mask = gtk_widget_get_events (widget) | 
		GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | 
		GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
		GDK_POINTER_MOTION_HINT_MASK;
	attributes.visual = gtk_widget_get_visual (widget);
		
	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;
	gtk_widget_set_window(widget, gdk_window_new(gtk_widget_get_parent_window(widget), &attributes, attributes_mask));
	
	gdk_window_set_user_data (gtk_widget_get_window(widget), widget);
}

static void gtk_tx_dial_get_preferred_width (GtkWidget *widget, gint *minimal_width, gint *natural_width) {
	*minimal_width = *natural_width = tX_knob_size;
}
static void gtk_tx_dial_get_preferred_height (GtkWidget *widget, gint *minimal_height, gint *natural_height) {
	*minimal_height = *natural_height = tX_knob_size;
}

static void gtk_tx_dial_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	GtkTxDial *tx_dial;
	
	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_TX_DIAL (widget));
	g_return_if_fail (allocation != NULL);
	
	gtk_widget_set_allocation(widget, allocation);
	tx_dial = GTK_TX_DIAL (widget);
	
	if (gtk_widget_get_realized (widget)) {
		gdk_window_move_resize (gtk_widget_get_window(widget),
				  allocation->x, allocation->y,
				  allocation->width, allocation->height);
		
		tx_dial->xofs=(allocation->width-tX_knob_size)/2;
		tx_dial->yofs=(allocation->height-tX_knob_size)/2;
	}
}

inline void gtk_tx_dial_do_draw (GtkTxDial *tx_dial, GtkWidget *widget, cairo_t *cr)
{
	if (gtk_widget_is_drawable (widget)) {
//		gdk_draw_pixbuf(gtk_widget_is_drawable (widget), 
//		                //gtk_widget_get_stlye(widget)->bg_gc[GTK_WIDGET_STATE(widget)],
//		                NULL, //TODO: this needs to be ported to cairo!
//		                knob_pixmaps[tx_dial->old_image],
//		                0, 0, tx_dial->xofs, tx_dial->yofs,
//						tX_knob_size, tX_knob_size, GDK_RGB_DITHER_NORMAL, 0, 0);

		gdk_cairo_set_source_pixbuf (cr, knob_pixmaps[tx_dial->old_image], 0, 0);
		cairo_paint (cr);
	}		 
}

gboolean gtk_tx_dial_draw (GtkWidget *widget, cairo_t *cr)
{
	GtkTxDial *tx_dial;
	
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_TX_DIAL (widget), FALSE);
	
	tx_dial = GTK_TX_DIAL (widget);
	
	gtk_tx_dial_do_draw(tx_dial, widget, cr);
		  
	return FALSE;
}

static gint gtk_tx_dial_button_press (GtkWidget *widget, GdkEventButton *event)
{
	GtkTxDial *tx_dial;
	
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_TX_DIAL (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);
	
	tx_dial = GTK_TX_DIAL (widget);
	
	tx_dial->x = event->x;
	tx_dial->y = event->y;    
	
	if (!tx_dial->button) {
		gtk_grab_add (widget);
		tx_dial->button = event->button;
	}
	
	return FALSE;
}

static gint gtk_tx_dial_button_release (GtkWidget *widget, GdkEventButton *event)
{
	GtkTxDial *tx_dial;
	
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_TX_DIAL (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);
	
	tx_dial = GTK_TX_DIAL (widget);
	
	if (tx_dial->button == event->button) {
		gtk_grab_remove (widget);
		tx_dial->button = 0;
	}
	
	return FALSE;
}

static gint gtk_tx_dial_motion_notify (GtkWidget *widget, GdkEventMotion *event)
{
	GtkTxDial *tx_dial;
	GdkModifierType mods;
	gint x, y, mask;
	
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_TX_DIAL (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);
	
	tx_dial = GTK_TX_DIAL (widget);
	
	if (tx_dial->button != 0) {
		x = event->x;
		y = event->y;
		
		if (event->is_hint || (event->window != gtk_widget_get_window(widget)))
			gdk_window_get_device_position(gtk_widget_get_window(widget), event->device, &x, &y, &mods);
		
		switch (tx_dial->button) {
			case 1:
				mask = GDK_BUTTON1_MASK;
				break;
			case 2:
				mask = GDK_BUTTON2_MASK;
				break;
			case 3:
				mask = GDK_BUTTON3_MASK;
				break;
			default:
				mask = 0;
		}
		
		if (mods & mask)
			gtk_tx_dial_update_mouse (tx_dial, x,y);
	}
	
	return FALSE;
}

static void gtk_tx_dial_update_mouse (GtkTxDial *tx_dial, gint x, gint y)
{
	gdouble dx, dy, d;
	gfloat old_value, new_value;

	g_return_if_fail (tx_dial != NULL);
	g_return_if_fail (GTK_IS_TX_DIAL (tx_dial));
	
	dx=x-tx_dial->x;
	dy=tx_dial->y-y;
	tx_dial->x=x;
	tx_dial->y=y;
	
	d=dx+dy;
	d/=200.0;
	
	old_value=gtk_adjustment_get_value(tx_dial->adjustment);    
	new_value=old_value + d*tx_dial->old_range;
	
	if (new_value>tx_dial->old_upper) 
		new_value=tx_dial->old_upper;
	else if (new_value<tx_dial->old_lower) 
		new_value=tx_dial->old_lower;
	
	gtk_adjustment_set_value(tx_dial->adjustment, new_value);
}

static void gtk_tx_dial_update (GtkTxDial *tx_dial)
{
	gfloat new_value;
	gint image;
	
	g_return_if_fail (tx_dial != NULL);
	g_return_if_fail (GTK_IS_TX_DIAL (tx_dial));
	
	new_value = gtk_adjustment_get_value(tx_dial->adjustment);
	
	if (new_value < gtk_adjustment_get_lower(tx_dial->adjustment))
		new_value = gtk_adjustment_get_lower(tx_dial->adjustment);
	
	if (new_value > gtk_adjustment_get_upper(tx_dial->adjustment))
		new_value = gtk_adjustment_get_upper(tx_dial->adjustment);
	
	if (new_value != gtk_adjustment_get_value(tx_dial->adjustment)) {
		gtk_adjustment_set_value(tx_dial->adjustment, new_value);
	}
	
	calc_image(new_value, image);
	
	if (image!=tx_dial->old_image) {
		tx_dial->old_image=image;
		gtk_widget_queue_draw(GTK_WIDGET(tx_dial));
	}
}

static void gtk_tx_dial_adjustment_changed (GtkAdjustment *adjustment,
			      gpointer       data)
{
	GtkTxDial *tx_dial;
	
	g_return_if_fail (adjustment != NULL);
	g_return_if_fail (data != NULL);
	
	tx_dial = GTK_TX_DIAL (data);
	
	if ((tx_dial->old_value != gtk_adjustment_get_value(adjustment)) ||
		(tx_dial->old_lower != gtk_adjustment_get_lower(adjustment)) ||
		(tx_dial->old_upper != gtk_adjustment_get_upper(adjustment))) {
		tx_dial->old_value = gtk_adjustment_get_value(adjustment);
		tx_dial->old_lower = gtk_adjustment_get_lower(adjustment);
		tx_dial->old_upper = gtk_adjustment_get_upper(adjustment);
		tx_dial->old_range = gtk_adjustment_get_upper(adjustment)-gtk_adjustment_get_lower(adjustment);
		
		gtk_tx_dial_update (tx_dial);
	}
}

static void gtk_tx_dial_adjustment_value_changed (GtkAdjustment *adjustment, gpointer data)
{
	GtkTxDial *tx_dial;
	
	g_return_if_fail (adjustment != NULL);
	g_return_if_fail (data != NULL);
	
	tx_dial = GTK_TX_DIAL (data);
	
	if (tx_dial->old_value != gtk_adjustment_get_value(adjustment)) {
		gtk_tx_dial_update (tx_dial);
		tx_dial->old_value = gtk_adjustment_get_value(adjustment);
	}
}
