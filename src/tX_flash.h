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
 
    File: tX_flash.h
 
    Description: Header to tX_flash.c
*/    


#ifndef __GTK_TX_FLASH_H__
#define __GTK_TX_FLASH_H__
#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>

#include "tX_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_TX_FLASH(obj)          GTK_CHECK_CAST (obj, gtk_tx_flash_get_type (), GtkTxFlash)
#define GTK_TX_FLASH_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, gtk_tx_flash_get_type (), GtkTxFlashClass)
#define GTK_IS_TX_FLASH(obj)       GTK_CHECK_TYPE (obj, gtk_tx_flash_get_type ())

typedef struct _GtkTxFlash        GtkTxFlash;
typedef struct _GtkTxFlashClass   GtkTxFlashClass;

struct _GtkTxFlash
{
        GtkWidget widget;
	
	GdkColor black;
	GdkColor red;
	GdkColor green;
	GdkColor lightgreen;
	GdkColor redgreen;
	
	int levels;
	int last_level;
	int red_level;
	f_prec level_value;
	
	int x1, x2;
	
	int max, max_cycles;
};

struct _GtkTxFlashClass
{
	GtkWidgetClass parent_class;
};

GtkWidget*     gtk_tx_flash_new	();
guint	       gtk_tx_flash_get_type (void);
static void           gtk_tx_flash_prepare (GtkWidget *widget);
void           gtk_tx_flash_set_level (GtkWidget *widget, f_prec new_value);
void           gtk_tx_flash_clear (GtkWidget *widget);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
