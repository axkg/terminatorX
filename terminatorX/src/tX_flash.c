/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2002  Alexander König
 
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

#define MAX_VALUE 32767
#define RED_BORDER 25000
#define TX_FLASH_DEFAULT_SIZE_X 17
#define TX_FLASH_DEFAULT_SIZE_Y 30
#define DY 5
#define DX 5
#define DMINIX 2
#define S_MINIX 2
#define L_MINIX 3
#define DLEVEL 3
#define MAX_MAX_CYCLES 20;

/* pre dec */
static void gtk_tx_flash_class_init (GtkTxFlashClass *);
static void gtk_tx_flash_init (GtkTxFlash *tx_flash);
GtkWidget* gtk_tx_flash_new ();
static void gtk_tx_flash_destroy (GtkObject *object);
static void gtk_tx_flash_realize (GtkWidget *widget);
static void gtk_tx_flash_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void gtk_tx_flash_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static gint gtk_tx_flash_expose (GtkWidget *widget, GdkEventExpose *event);
//static void gtk_tx_flash_prepare (GtkWidget *widget);
//static void gtk_tx_flash_set_level(GtkWidget *widget, f_prec new_value);

/* Local data */

static GtkWidgetClass *parent_class = NULL;

guint
gtk_tx_flash_get_type ()
{
  static guint tx_flash_type = 0;

  if (!tx_flash_type)
    {
      GtkTypeInfo tx_flash_info =
      {
	"GtkTxFlash",
	sizeof (GtkTxFlash),
	sizeof (GtkTxFlashClass),
	(GtkClassInitFunc) gtk_tx_flash_class_init,
	(GtkObjectInitFunc) gtk_tx_flash_init,
	(GtkArgSetFunc) NULL,
	(GtkArgGetFunc) NULL,
      };

      tx_flash_type = gtk_type_unique (gtk_widget_get_type (), &tx_flash_info);
    }

  return tx_flash_type;
}

static void
gtk_tx_flash_class_init (GtkTxFlashClass *gclass)
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
//  widget_class->button_press_event = gtk_tx_flash_button_press;
//  widget_class->button_release_event = gtk_tx_flash_button_release;
//  widget_class->motion_notify_event = gtk_tx_flash_motion_notify;
}

void gtk_tx_flash_mk_col(GtkTxFlash *tx_flash, GdkColor *col, float r, float g, float b)
{
	float max=65535.0;
	
	col->red=(gint) (r*max);
	col->green=(gint) (g*max);
	col->blue=(gint) (b*max);
	gdk_colormap_alloc_color (gtk_widget_get_colormap (GTK_WIDGET(tx_flash)), col, 1, 1);		
}

static void
gtk_tx_flash_init (GtkTxFlash *tx_flash)
{
	GdkColormap *priv;
			
	priv=gdk_colormap_new(gtk_widget_get_visual(GTK_WIDGET(tx_flash)), 6);

	gtk_widget_set_colormap(GTK_WIDGET(tx_flash), priv);

	gtk_tx_flash_mk_col(tx_flash, &tx_flash->black, 0, 0, 0);
	gtk_tx_flash_mk_col(tx_flash, &tx_flash->red, 1, 0.5, 0.5);
	gtk_tx_flash_mk_col(tx_flash, &tx_flash->green, 0.5, 1, 0.5);	
	gtk_tx_flash_mk_col(tx_flash, &tx_flash->lightgreen, 0, 0.7, 0);
	gtk_tx_flash_mk_col(tx_flash, &tx_flash->redgreen, 0.7, 0, 0);
}

GtkWidget*
gtk_tx_flash_new ()
{
  GtkTxFlash *tx_flash;

  tx_flash = gtk_type_new (gtk_tx_flash_get_type ());

  return GTK_WIDGET (tx_flash);
}

static void
gtk_tx_flash_destroy (GtkObject *object)
{
  g_return_if_fail (object != NULL);
  g_return_if_fail (GTK_IS_TX_FLASH (object));

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
gtk_tx_flash_realize (GtkWidget *widget)
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

static void 
gtk_tx_flash_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	requisition->width = TX_FLASH_DEFAULT_SIZE_X;
	requisition->height = TX_FLASH_DEFAULT_SIZE_Y;
}

static void
gtk_tx_flash_prepare (GtkWidget *widget)
{
	GtkTxFlash *tx_flash;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_TX_FLASH (widget));
	
	tx_flash=GTK_TX_FLASH(widget);

	tx_flash->levels=(widget->allocation.height-(2*DY))/DLEVEL;
	tx_flash->last_level=0;
	tx_flash->level_value=MAX_VALUE/(f_prec) tx_flash->levels;
	tx_flash->red_level=(RED_BORDER/tx_flash->level_value);
	
	tx_flash->x1=DMINIX+S_MINIX+2;
	tx_flash->x2=widget->allocation.width-tx_flash->x1-1;
	tx_flash->max=0;
}

static void
gtk_tx_flash_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
  GtkTxFlash *tx_flash;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_TX_FLASH (widget));
  g_return_if_fail (allocation != NULL);

  widget->allocation = *allocation;

  gtk_tx_flash_prepare(widget);

  if (GTK_WIDGET_REALIZED (widget))
    {
      tx_flash = GTK_TX_FLASH (widget);

      gdk_window_move_resize (widget->window,
			      allocation->x, allocation->y,
			      allocation->width, allocation->height);

    }
}

static void gtk_tx_flash_paint_yourself(GtkWidget *widget)
{
  GtkTxFlash *tx_flash;
  gint i, x11, x12,x21, x22, y;
  int long_level;

  tx_flash = GTK_TX_FLASH (widget);

  gdk_gc_set_foreground(widget->style->fg_gc[widget->state], &tx_flash->black);
  
  gdk_draw_rectangle(widget->window, widget->style->fg_gc[widget->state], 1, 0, 0, widget->allocation.width,widget->allocation.height); 

  gdk_gc_set_foreground(widget->style->fg_gc[widget->state], &tx_flash->lightgreen);

  x12=DMINIX+S_MINIX;
  x21=widget->allocation.width-1-x12;

  for (i=0, y=widget->allocation.height-DY; i<=tx_flash->levels; y-=DLEVEL, i++)
  {
  	if (i==0)
	{
		long_level=1;
	}
	else if (i==tx_flash->red_level-1)
	{
		long_level=1;
	}
	else if (i==tx_flash->red_level)
	{
		long_level=1;
		gdk_gc_set_foreground(widget->style->fg_gc[widget->state], &tx_flash->redgreen);
	}
	else if (i==tx_flash->levels)
	{
		long_level=1;
	}
	else long_level=0;
	
  	if (long_level)
	{
		x11=x12-L_MINIX;
		x22=x21+L_MINIX;
	}
	else
	{
		x11=x12-S_MINIX;
		x22=x21+S_MINIX;		
	}
	
	gdk_draw_line(widget->window, widget->style->fg_gc[widget->state], x11, y, x12, y);
	gdk_draw_line(widget->window, widget->style->fg_gc[widget->state], x21, y, x22, y);
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

static void
gtk_tx_flash_update (GtkTxFlash *tx_flash)
{
  g_return_if_fail (tx_flash != NULL);
  g_return_if_fail (GTK_IS_TX_FLASH (tx_flash));

  gtk_widget_draw (GTK_WIDGET(tx_flash), NULL);
}

void
gtk_tx_flash_set_level(GtkWidget *widget, f_prec new_value)
{
  GtkTxFlash *tx_flash;
  gint i, y;
  int new_level, end_level;
  int red=0;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_TX_FLASH (widget));

  tx_flash = GTK_TX_FLASH (widget);

  new_level=(int) (new_value/tx_flash->level_value);
  
  if (new_level>tx_flash->levels) new_level=tx_flash->levels;
  
//  printf ("%f, %i, %i\n", tx_flash->level_value,new_level, tx_flash->last_level);
  

  if (new_level>tx_flash->max)
  {
  	tx_flash->max=new_level;
	tx_flash->max_cycles=MAX_MAX_CYCLES;
  }
  else
  {
  	tx_flash->max_cycles--;
  }
  
  if (tx_flash->max_cycles <= 0)
  {  	
	y=widget->allocation.height-(DY+(tx_flash->max)*DLEVEL);
	gdk_gc_set_foreground(widget->style->fg_gc[widget->state], &tx_flash->black);
	gdk_draw_line(widget->window, widget->style->fg_gc[widget->state], tx_flash->x1, y, tx_flash->x2, y);

	if (tx_flash->max>0)
	{
		tx_flash->max--;
		y+=DLEVEL;
		if (tx_flash->max>tx_flash->red_level)
		{
			gdk_gc_set_foreground(widget->style->fg_gc[widget->state], &tx_flash->red);		
		}
		else
		{
			gdk_gc_set_foreground(widget->style->fg_gc[widget->state], &tx_flash->green);		
		}
		gdk_draw_line(widget->window, widget->style->fg_gc[widget->state], tx_flash->x1, y, tx_flash->x2, y);	
	}
  }

  if (new_level==tx_flash->last_level) return;

  if (new_level<tx_flash->last_level) // make it look more realistic
  {
  	new_level=tx_flash->last_level*globals.flash_response;
  }
  
  if (new_level>tx_flash->last_level)
  {
	  gdk_gc_set_foreground(widget->style->fg_gc[widget->state], &tx_flash->green);
  
  	  for (i=tx_flash->last_level, y=widget->allocation.height-(DY+tx_flash->last_level*DLEVEL); i<=new_level; y-=DLEVEL, i++)
	  {
	  	if (!red)
		{
			if (i>=tx_flash->red_level)
			{
				gdk_gc_set_foreground(widget->style->fg_gc[widget->state], &tx_flash->red);
				red=1;
			}
		}
		gdk_draw_line(widget->window, widget->style->fg_gc[widget->state], tx_flash->x1, y, tx_flash->x2, y);
	  }
  }
  else
  {
	  gdk_gc_set_foreground(widget->style->fg_gc[widget->state], &tx_flash->black);

	  if (tx_flash->last_level==tx_flash->max)
	  {
	  	i=tx_flash->last_level-1;
	  }
	  else
	  {
		i=tx_flash->last_level;
	  }

  	  for (y=widget->allocation.height-(DY+i*DLEVEL); i>new_level; y+=DLEVEL, i--)
	  {
		gdk_draw_line(widget->window, widget->style->fg_gc[widget->state], tx_flash->x1, y, tx_flash->x2, y);
	  }
  }
  tx_flash->last_level=new_level;
}

void
gtk_tx_flash_clear (GtkWidget *widget)
{
	GtkTxFlash *tx_flash;

	tx_flash = GTK_TX_FLASH (widget);
	
	tx_flash->max=0;
	tx_flash->max_cycles=0;
	tx_flash->last_level=0;
	
	gtk_tx_flash_paint_yourself(widget);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
