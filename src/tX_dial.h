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
 
    File: tX_dial.h
 
    Description: Header to tX_dial.c - this widget is based on the gtk_dial
    example from the gtk+ tutorial which is
    Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
*/    

#ifndef __GTK_TX_DIAL_H__
#define __GTK_TX_DIAL_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GTK_TX_DIAL(obj)	       G_TYPE_CHECK_INSTANCE_CAST (obj, gtk_tx_dial_get_type (), GtkTxDial)
#define GTK_TX_DIAL_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, gtk_tx_dial_get_type (), GtkTxDialClass)
#define GTK_IS_TX_DIAL(obj)       G_TYPE_CHECK_INSTANCE_TYPE (obj, gtk_tx_dial_get_type ())

typedef struct _GtkTxDial        GtkTxDial;
typedef struct _GtkTxDialClass   GtkTxDialClass;

struct _GtkTxDial {
	GtkWidget widget;
	
	/* Button currently pressed or 0 if none */
	guint8 button;
	
	/* Old values from adjustment stored so we know when something changes */
	gfloat old_value;
	gfloat old_lower;
	gfloat old_upper;
	gfloat old_range;
	
	/* The adjustment object that stores the data for this tx_dial */
	GtkAdjustment *adjustment;
	
	gint x, y;
	gint xofs, yofs;
	
	gint old_image;
};

struct _GtkTxDialClass {
	GtkWidgetClass parent_class;
};

GtkWidget* gtk_tx_dial_new (GtkAdjustment *adjustment);
GType gtk_tx_dial_get_type (void);
GtkAdjustment* gtk_tx_dial_get_adjustment (GtkTxDial *tx_dial);

void gtk_tx_dial_set_adjustment (GtkTxDial *tx_dial, GtkAdjustment *adjustment);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GTK_TX_DIAL_H__ */
