/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2022  Alexander König

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

    File: tX_knobloader.c

    Description: This code loads the knob-images required for tX_dial widget.
*/

#include "tX_knobloader.h"
#include "icons/tX_knob_resources.c"
#include "tX_global.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>

#ifdef USE_DIAL
int tX_knob_size;
GdkPixbuf* knob_pixmaps[MAX_KNOB_PIX];

void load_knob_pixs(int fontHeight, int scaleFactor) {
    int i;
    GError* error = NULL;

    g_resource_new_from_data(g_bytes_new_static(tX_knob_resource_data.data, sizeof(tX_knob_resource_data.data)), &error);
    if (error) {
        tX_error("failed accessing tX_dial resources: %s\n", error->message);
    }

    if (globals.knob_size_override > 0) {
        tX_knob_size = globals.knob_size_override;
    } else {
        tX_knob_size = fontHeight * 3 * scaleFactor;
    }
    tX_debug("load_knob_pix(): knob size is %i", tX_knob_size);

    for (i = 0; i < MAX_KNOB_PIX; i++) {
        char resource_path[256];
        snprintf(resource_path, 256, "/org/terminatorX/tX_dial/knob%i.png", i);
        knob_pixmaps[i] = gdk_pixbuf_new_from_resource_at_scale(resource_path, tX_knob_size, tX_knob_size, TRUE, &error);

        if (error) {
            tX_error("failed rendering knob image: %s\n", error->message);
        }
    }
}

#endif
