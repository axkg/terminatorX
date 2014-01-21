/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2014  Alexander König
 
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
static void gtk_tx_dial_destroy			(GtkObject *object);
static void gtk_tx_dial_realize			(GtkWidget *widget);
static void gtk_tx_dial_size_request	(GtkWidget *widget, GtkRequisition *requisition);
static void gtk_tx_dial_size_allocate	(GtkWidget *widget, GtkAllocation *allocation);
static gint gtk_tx_dial_expose			(GtkWidget *widget, GdkEventExpose *event);
static gint gtk_tx_dial_button_press	(GtkWidget *widget, GdkEventButton *event);
static gint gtk_tx_dial_button_release	(GtkWidget *widget, GdkEventButton *event);
static gint gtk_tx_dial_motion_notify	(GtkWidget *widget, GdkEventMotion *event);
static gint gtk_tx_dial_timer			(GtkTxDial *tx_dial);
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
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass*) class;
	widget_class = (GtkWidgetClass*) class;

	parent_class = (GtkWidgetClass*) g_type_class_peek (gtk_widget_get_type ());

	object_class->destroy = gtk_tx_dial_destroy;

	widget_class->realize = gtk_tx_dial_realize;
	widget_class->expose_event = gtk_tx_dial_expose;
	widget_class->size_request = gtk_tx_dial_size_request;
	widget_class->size_allocate = gtk_tx_dial_size_allocate;
	widget_class->button_press_event = gtk_tx_dial_button_press;
	widget_class->button_release_event = gtk_tx_dial_button_release;
	widget_class->motion_notify_event = gtk_tx_dial_motion_notify;
}

static void gtk_tx_dial_init (GtkTxDial *tx_dial)
{
	tx_dial->button = 0;
	tx_dial->policy = GTK_UPDATE_CONTINUOUS;
	tx_dial->timer = 0;

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

static void gtk_tx_dial_destroy (GtkObject *object)
{
	GtkTxDial *tx_dial;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_IS_TX_DIAL (object));
	
	tx_dial = GTK_TX_DIAL (object);

	if (tx_dial->adjustment)
		g_object_unref (G_OBJECT (tx_dial->adjustment));
	
	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(*GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

GtkAdjustment* gtk_tx_dial_get_adjustment (GtkTxDial *tx_dial)
{
	g_return_val_if_fail (tx_dial != NULL, NULL);
	g_return_val_if_fail (GTK_IS_TX_DIAL (tx_dial), NULL);
	
	return tx_dial->adjustment;
}

void gtk_tx_dial_set_update_policy (GtkTxDial *tx_dial, GtkUpdateType policy)
{
	g_return_if_fail (tx_dial != NULL);
	g_return_if_fail (GTK_IS_TX_DIAL (tx_dial));

	tx_dial->policy = policy;
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
	attributes.colormap = gtk_widget_get_colormap (widget);
	
	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
	gtk_widget_set_window(widget, gdk_window_new(gtk_widget_get_parent_window(widget), &attributes, attributes_mask));
	
	gdk_window_set_user_data (gtk_widget_get_window(widget), widget);
	gtk_widget_set_style(widget, gtk_style_attach (gtk_widget_get_style(widget), gtk_widget_get_window(widget)));
	gtk_style_set_background (gtk_widget_get_style(widget), gtk_widget_get_window(widget), GTK_STATE_NORMAL);
}

static void gtk_tx_dial_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
	requisition->width = KNOB_SIZE;
	requisition->height = KNOB_SIZE;
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
		
		tx_dial->xofs=(allocation->width-KNOB_SIZE)/2;
		tx_dial->yofs=(allocation->height-KNOB_SIZE)/2;
	}
}

inline void gtk_tx_dial_draw (GtkTxDial *tx_dial, GtkWidget *widget)
{
	if (gtk_widget_is_drawable (widget)) {
//		gdk_draw_pixbuf(gtk_widget_is_drawable (widget), 
//		                //gtk_widget_get_stlye(widget)->bg_gc[GTK_WIDGET_STATE(widget)],
//		                NULL, //TODO: this needs to be ported to cairo!
//		                knob_pixmaps[tx_dial->old_image],
//		                0, 0, tx_dial->xofs, tx_dial->yofs,
//						KNOB_SIZE, KNOB_SIZE, GDK_RGB_DITHER_NORMAL, 0, 0);

		cairo_t *cr = gdk_cairo_create (gtk_widget_get_window(widget));
		gdk_cairo_set_source_pixbuf (cr, knob_pixmaps[tx_dial->old_image], 0, 0);
		cairo_paint (cr);
		cairo_destroy (cr);
	}		 
}

static gint gtk_tx_dial_expose (GtkWidget *widget, GdkEventExpose *event)
{
	GtkTxDial *tx_dial;
	
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_TX_DIAL (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);
	
	if (event->count > 0)
	return FALSE;
	
	tx_dial = GTK_TX_DIAL (widget);
	
	gtk_tx_dial_draw(tx_dial, widget);
		  
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
		
		if (tx_dial->policy == GTK_UPDATE_DELAYED)
			g_source_remove (tx_dial->timer);
		
		if ((tx_dial->policy != GTK_UPDATE_CONTINUOUS) &&
			(tx_dial->old_value != gtk_adjustment_get_value(tx_dial->adjustment)))
			g_signal_emit_by_name (G_OBJECT (tx_dial->adjustment),
			"value_changed");
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
			gdk_window_get_pointer (gtk_widget_get_window(widget), &x, &y, &mods);
		
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

static gint gtk_tx_dial_timer (GtkTxDial *tx_dial)
{
	g_return_val_if_fail (tx_dial != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_TX_DIAL (tx_dial), FALSE);
	
	if (tx_dial->policy == GTK_UPDATE_DELAYED)
		g_signal_emit_by_name (G_OBJECT (tx_dial->adjustment),
				 "value_changed");
	
	return FALSE;
}

static void gtk_tx_dial_update_mouse (GtkTxDial *tx_dial, gint x, gint y)
{
	gdouble dx, dy, d;
	gfloat old_value, new_value;
	gint image;
	
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
	
	if (gtk_adjustment_get_value(tx_dial->adjustment) != old_value) {
		if (tx_dial->policy == GTK_UPDATE_CONTINUOUS)	{
			g_signal_emit_by_name (G_OBJECT (tx_dial->adjustment),
				   "value_changed");
		} else {
			calc_image(gtk_adjustment_get_value(tx_dial->adjustment), image);
		
			if (image!=tx_dial->old_image) {
		 		tx_dial->old_image=image;
				gtk_widget_queue_draw(GTK_WIDGET(tx_dial));
			}
		
			if (tx_dial->policy == GTK_UPDATE_DELAYED) {
		  		if (tx_dial->timer)
					g_source_remove (tx_dial->timer);
		
					tx_dial->timer = g_timeout_add (SCROLL_DELAY_LENGTH,
						 (GSourceFunc) gtk_tx_dial_timer,
						 (gpointer) tx_dial);
			}
		}
	}
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
		g_signal_emit_by_name (G_OBJECT (tx_dial->adjustment), "value_changed");
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
