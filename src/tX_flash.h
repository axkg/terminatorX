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
 
    File: tX_flash.h
 
    Description: Header to tX_flash.c
*/    


#ifndef __GTK_TX_FLASH_H__
#define __GTK_TX_FLASH_H__
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "tX_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_TX_FLASH(obj) G_TYPE_CHECK_INSTANCE_CAST (obj, gtk_tx_flash_get_type(), GtkTxFlash)
#define GTK_TX_FLASH_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gtk_tx_flash_get_type(), GtkTxFlashClass)
#define GTK_IS_TX_FLASH(obj) G_TYPE_CHECK_INSTANCE_TYPE (obj, gtk_tx_flash_get_type())

typedef struct _GtkTxFlash GtkTxFlash;
typedef struct _GtkTxFlashClass GtkTxFlashClass;

struct flash_channel {
	int last_level;
	int max;
	int x1;
	int x2;
};

struct _GtkTxFlash {
	GtkWidget widget;
	GdkRGBA colors[5];	
	int levels;
	int red_level;
	f_prec level_value;
	int max_cycles;
	int center_expand;
	struct flash_channel channel[2];
	cairo_surface_t* surface;
};

struct _GtkTxFlashClass {
	GtkWidgetClass parent_class;
};

GtkWidget* gtk_tx_flash_new();
GType gtk_tx_flash_get_type (void);
void gtk_tx_flash_set_level (GtkWidget *widget, f_prec left_channel, f_prec right_channel);
void gtk_tx_flash_clear (GtkWidget *widget);
void gtk_tx_flash_update_colors(GtkTxFlash *tx);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
