/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999, 2000  Alexander König
 
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

#ifdef USE_DIAL

#include <math.h>
#include <stdio.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include "tX_knobloader.h"

#include "tX_dial.h"

#define SCROLL_DELAY_LENGTH  300
#define TX_DIAL_DEFAULT_SIZE 100

/* Forward declarations */

static void gtk_tx_dial_class_init 	      (GtkTxDialClass    *klass);
static void gtk_tx_dial_init		      (GtkTxDial         *tx_dial);
static void gtk_tx_dial_destroy		      (GtkObject	*object);
static void gtk_tx_dial_realize		      (GtkWidget	*widget);
static void gtk_tx_dial_size_request	      (GtkWidget      *widget,
					       GtkRequisition *requisition);
static void gtk_tx_dial_size_allocate	      (GtkWidget     *widget,
					       GtkAllocation *allocation);
static gint gtk_tx_dial_expose		      (GtkWidget	*widget,
						GdkEventExpose   *event);
static gint gtk_tx_dial_button_press	      (GtkWidget	*widget,
						GdkEventButton   *event);
static gint gtk_tx_dial_button_release	      (GtkWidget	*widget,
						GdkEventButton   *event);
static gint gtk_tx_dial_motion_notify	      (GtkWidget	*widget,
						GdkEventMotion   *event);
static gint gtk_tx_dial_timer		      (GtkTxDial         *tx_dial);

static void gtk_tx_dial_update_mouse	      (GtkTxDial *tx_dial, gint x, gint y);
static void gtk_tx_dial_update		      (GtkTxDial *tx_dial);
static void gtk_tx_dial_adjustment_changed       (GtkAdjustment	*adjustment,
						gpointer	  data);
static void gtk_tx_dial_adjustment_value_changed (GtkAdjustment	*adjustment,
						gpointer	  data);

/* Local data */

static GtkWidgetClass *parent_class = NULL;

#define calc_image(f,i); i=(gint) ((f - tx_dial->old_lower)/(tx_dial->old_range) * ((float) TX_MAX_KNOB_PIX)); if(i>TX_MAX_KNOB_PIX) i=TX_MAX_KNOB_PIX; else if (i<0) i=0;

guint
gtk_tx_dial_get_type ()
{
  static guint tx_dial_type = 0;

  if (!tx_dial_type)
    {
      GtkTypeInfo tx_dial_info =
      {
	"GtkTxDial",
	sizeof (GtkTxDial),
	sizeof (GtkTxDialClass),
	(GtkClassInitFunc) gtk_tx_dial_class_init,
	(GtkObjectInitFunc) gtk_tx_dial_init,
	(GtkArgSetFunc) NULL,
	(GtkArgGetFunc) NULL,
      };

      tx_dial_type = gtk_type_unique (gtk_widget_get_type (), &tx_dial_info);
    }

  return tx_dial_type;
}

static void
gtk_tx_dial_class_init (GtkTxDialClass *class)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = (GtkObjectClass*) class;
  widget_class = (GtkWidgetClass*) class;

  parent_class = gtk_type_class (gtk_widget_get_type ());

  object_class->destroy = gtk_tx_dial_destroy;

  widget_class->realize = gtk_tx_dial_realize;
  widget_class->expose_event = gtk_tx_dial_expose;
  widget_class->size_request = gtk_tx_dial_size_request;
  widget_class->size_allocate = gtk_tx_dial_size_allocate;
  widget_class->button_press_event = gtk_tx_dial_button_press;
  widget_class->button_release_event = gtk_tx_dial_button_release;
  widget_class->motion_notify_event = gtk_tx_dial_motion_notify;
}

static void
gtk_tx_dial_init (GtkTxDial *tx_dial)
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

GtkWidget*
gtk_tx_dial_new (GtkAdjustment *adjustment)
{
  GtkTxDial *tx_dial;

  tx_dial = gtk_type_new (gtk_tx_dial_get_type ());

  if (!adjustment)
    adjustment = (GtkAdjustment*) gtk_adjustment_new (0.0, 0.0, 0.0,
						      0.0, 0.0, 0.0);

  gtk_tx_dial_set_adjustment (tx_dial, adjustment);

  return GTK_WIDGET (tx_dial);
}

static void
gtk_tx_dial_destroy (GtkObject *object)
{
  GtkTxDial *tx_dial;

  g_return_if_fail (object != NULL);
  g_return_if_fail (GTK_IS_TX_DIAL (object));

  tx_dial = GTK_TX_DIAL (object);

  if (tx_dial->adjustment)
    gtk_object_unref (GTK_OBJECT (tx_dial->adjustment));

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

GtkAdjustment*
gtk_tx_dial_get_adjustment (GtkTxDial *tx_dial)
{
  g_return_val_if_fail (tx_dial != NULL, NULL);
  g_return_val_if_fail (GTK_IS_TX_DIAL (tx_dial), NULL);

  return tx_dial->adjustment;
}

void
gtk_tx_dial_set_update_policy (GtkTxDial	 *tx_dial,
			     GtkUpdateType  policy)
{
  g_return_if_fail (tx_dial != NULL);
  g_return_if_fail (GTK_IS_TX_DIAL (tx_dial));

  tx_dial->policy = policy;
}

void
gtk_tx_dial_set_adjustment (GtkTxDial      *tx_dial,
			  GtkAdjustment *adjustment)
{
  g_return_if_fail (tx_dial != NULL);
  g_return_if_fail (GTK_IS_TX_DIAL (tx_dial));

  if (tx_dial->adjustment)
    {
      gtk_signal_disconnect_by_data (GTK_OBJECT (tx_dial->adjustment),
				     (gpointer) tx_dial);
      gtk_object_unref (GTK_OBJECT (tx_dial->adjustment));
    }

  tx_dial->adjustment = adjustment;
  gtk_object_ref (GTK_OBJECT (tx_dial->adjustment));

  gtk_signal_connect (GTK_OBJECT (adjustment), "changed",
		      (GtkSignalFunc) gtk_tx_dial_adjustment_changed,
		      (gpointer) tx_dial);
  gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed",
		      (GtkSignalFunc) gtk_tx_dial_adjustment_value_changed,
		      (gpointer) tx_dial);

  tx_dial->old_value = adjustment->value;
  tx_dial->old_lower = adjustment->lower;
  tx_dial->old_upper = adjustment->upper;
  tx_dial->old_range = adjustment->upper - adjustment->lower;
  
  calc_image(adjustment->value,tx_dial->old_image);

  gtk_tx_dial_update (tx_dial);
}

static void
gtk_tx_dial_realize (GtkWidget *widget)
{
  GtkTxDial *tx_dial;
  GdkWindowAttr attributes;
  gint attributes_mask;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_TX_DIAL (widget));

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
  tx_dial = GTK_TX_DIAL (widget);

  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.event_mask = gtk_widget_get_events (widget) | 
    GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | 
    GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
    GDK_POINTER_MOTION_HINT_MASK;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
  widget->window = gdk_window_new (widget->parent->window,
				   &attributes,
				   attributes_mask);

  widget->style = gtk_style_attach (widget->style, widget->window);

  gdk_window_set_user_data (widget->window, widget);

//  gtk_widget_set_style(widget, gtk_widget_get_default_style());
  gtk_style_set_background (widget->style, widget->window, GTK_WIDGET_STATE (widget));
}

static void 
gtk_tx_dial_size_request (GtkWidget      *widget,
		       GtkRequisition *requisition)
{
  requisition->width = KNOB_SIZE;
  requisition->height = KNOB_SIZE;
}

static void
gtk_tx_dial_size_allocate (GtkWidget     *widget,
			GtkAllocation *allocation)
{
  GtkTxDial *tx_dial;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_TX_DIAL (widget));
  g_return_if_fail (allocation != NULL);

  widget->allocation = *allocation;
  tx_dial = GTK_TX_DIAL (widget);

  if (GTK_WIDGET_REALIZED (widget))
    {

      gdk_window_move_resize (widget->window,
			      allocation->x, allocation->y,
			      allocation->width, allocation->height);
 
     tx_dial->xofs=(allocation->width-KNOB_SIZE)/2;
     tx_dial->yofs=(allocation->height-KNOB_SIZE)/2;
    }
}

inline void
gtk_tx_dial_draw (GtkTxDial *tx_dial, GtkWidget *widget)
{
	if (GTK_WIDGET_DRAWABLE (widget))
	gdk_draw_pixmap(widget->window, widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
                 knob_pixmaps[tx_dial->old_image], 0, 0, tx_dial->xofs, tx_dial->yofs, KNOB_SIZE, KNOB_SIZE);
}

static gint
gtk_tx_dial_expose (GtkWidget	*widget,
		 GdkEventExpose *event)
{
  GtkTxDial *tx_dial;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_TX_DIAL (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  if (event->count > 0)
    return FALSE;
  
  tx_dial = GTK_TX_DIAL (widget);

/*  gdk_window_clear_area (widget->window,
			 0, 0,
			 widget->allocation.width,
			 widget->allocation.height);
gdk_draw_rectangle(widget->window, widget->style->fg_gc[widget->state], 1, 0, 0,
			 widget->allocation.width,
			 widget->allocation.height);*/

  gtk_tx_dial_draw(tx_dial, widget);
		  
  return FALSE;
}

static gint
gtk_tx_dial_button_press (GtkWidget      *widget,
		       GdkEventButton *event)
{
  GtkTxDial *tx_dial;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_TX_DIAL (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  tx_dial = GTK_TX_DIAL (widget);

  tx_dial->x = event->x;
  tx_dial->y = event->y;    
  
  if (!tx_dial->button)
    {
      gtk_grab_add (widget);

      tx_dial->button = event->button;
    }

  return FALSE;
}

static gint
gtk_tx_dial_button_release (GtkWidget	*widget,
			  GdkEventButton *event)
{
  GtkTxDial *tx_dial;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_TX_DIAL (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  tx_dial = GTK_TX_DIAL (widget);

  if (tx_dial->button == event->button)
    {
      gtk_grab_remove (widget);

      tx_dial->button = 0;

      if (tx_dial->policy == GTK_UPDATE_DELAYED)
	gtk_timeout_remove (tx_dial->timer);
      
      if ((tx_dial->policy != GTK_UPDATE_CONTINUOUS) &&
	  (tx_dial->old_value != tx_dial->adjustment->value))
	gtk_signal_emit_by_name (GTK_OBJECT (tx_dial->adjustment),
				 "value_changed");
    }

  return FALSE;
}

static gint
gtk_tx_dial_motion_notify (GtkWidget      *widget,
			 GdkEventMotion *event)
{
  GtkTxDial *tx_dial;
  GdkModifierType mods;
  gint x, y, mask;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_TX_DIAL (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  tx_dial = GTK_TX_DIAL (widget);

  if (tx_dial->button != 0)
    {
      x = event->x;
      y = event->y;

      if (event->is_hint || (event->window != widget->window))
	gdk_window_get_pointer (widget->window, &x, &y, &mods);

      switch (tx_dial->button)
	{
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
	  break;
	}

      if (mods & mask)
	gtk_tx_dial_update_mouse (tx_dial, x,y);
    }

  return FALSE;
}

static gint
gtk_tx_dial_timer (GtkTxDial *tx_dial)
{
  g_return_val_if_fail (tx_dial != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_TX_DIAL (tx_dial), FALSE);

  if (tx_dial->policy == GTK_UPDATE_DELAYED)
    gtk_signal_emit_by_name (GTK_OBJECT (tx_dial->adjustment),
			     "value_changed");

  return FALSE;
}

static void
gtk_tx_dial_update_mouse (GtkTxDial *tx_dial, gint x, gint y)
{
  gdouble dx, dy, d;
  gfloat old_value;
  gfloat new_value;
  gint image;

  g_return_if_fail (tx_dial != NULL);
  g_return_if_fail (GTK_IS_TX_DIAL (tx_dial));

  dx=x-tx_dial->x;
  dy=tx_dial->y-y;

  tx_dial->x=x;
  tx_dial->y=y;

  //if (fabs(dx) > fabs(dy)) d=dx; else d=dy;
  d=dx+dy;
  
  d/=200.0;

  old_value=tx_dial->adjustment->value;    
    
  new_value=old_value + d*tx_dial->old_range;
  
  if (new_value>tx_dial->old_upper) new_value=tx_dial->old_upper;
  else if (new_value<tx_dial->old_lower) new_value=tx_dial->old_lower;
 
  tx_dial->adjustment->value=new_value;
    
  if (tx_dial->adjustment->value != old_value)
    {
      if (tx_dial->policy == GTK_UPDATE_CONTINUOUS)
	{
	  gtk_signal_emit_by_name (GTK_OBJECT (tx_dial->adjustment),
				   "value_changed");
	}
      else
	{
  	  calc_image(tx_dial->adjustment->value, image);

	  if (image!=tx_dial->old_image)
	  {
		 tx_dial->old_image=image;
//	  	 gtk_widget_draw (GTK_WIDGET(tx_dial), NULL);
  		 gtk_tx_dial_draw(tx_dial, GTK_WIDGET(tx_dial));
	  }

	  if (tx_dial->policy == GTK_UPDATE_DELAYED)
	    {
	      if (tx_dial->timer)
		gtk_timeout_remove (tx_dial->timer);

	      tx_dial->timer = gtk_timeout_add (SCROLL_DELAY_LENGTH,
					     (GtkFunction) gtk_tx_dial_timer,
					     (gpointer) tx_dial);
	    }
	}
    }
}

static void
gtk_tx_dial_update (GtkTxDial *tx_dial)
{
  gfloat new_value;
  gint image;
  
  g_return_if_fail (tx_dial != NULL);
  g_return_if_fail (GTK_IS_TX_DIAL (tx_dial));

  new_value = tx_dial->adjustment->value;
  
  if (new_value < tx_dial->adjustment->lower)
    new_value = tx_dial->adjustment->lower;

  if (new_value > tx_dial->adjustment->upper)
    new_value = tx_dial->adjustment->upper;

  if (new_value != tx_dial->adjustment->value)
    {
      tx_dial->adjustment->value = new_value;
      gtk_signal_emit_by_name (GTK_OBJECT (tx_dial->adjustment), "value_changed");
    }
    
     calc_image(new_value, image);
     if (image!=tx_dial->old_image)
     {
     		 tx_dial->old_image=image;
  		 gtk_tx_dial_draw(tx_dial, GTK_WIDGET(tx_dial));
     }

//  gtk_widget_draw (GTK_WIDGET(tx_dial), NULL);
}

static void
gtk_tx_dial_adjustment_changed (GtkAdjustment *adjustment,
			      gpointer       data)
{
  GtkTxDial *tx_dial;

  g_return_if_fail (adjustment != NULL);
  g_return_if_fail (data != NULL);

  tx_dial = GTK_TX_DIAL (data);

  if ((tx_dial->old_value != adjustment->value) ||
      (tx_dial->old_lower != adjustment->lower) ||
      (tx_dial->old_upper != adjustment->upper))
    {
      tx_dial->old_value = adjustment->value;
      tx_dial->old_lower = adjustment->lower;
      tx_dial->old_upper = adjustment->upper;
      tx_dial->old_range = adjustment->upper-adjustment->lower;
      
//      calc_image(adjustment->value, tx_dial->old_image)
//  		 gtk_tx_dial_draw(tx_dial, GTK_WIDET(tx_dial));
      gtk_tx_dial_update (tx_dial);
      
    }
}

static void
gtk_tx_dial_adjustment_value_changed (GtkAdjustment *adjustment,
				    gpointer	   data)
{
  GtkTxDial *tx_dial;
  gint image;

  g_return_if_fail (adjustment != NULL);
  g_return_if_fail (data != NULL);

  tx_dial = GTK_TX_DIAL (data);

  if (tx_dial->old_value != adjustment->value)
    {
      gtk_tx_dial_update (tx_dial);

      tx_dial->old_value = adjustment->value;
    }
}
/* example-end */
#endif
