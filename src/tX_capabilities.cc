/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2003  Alexander König
 
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
 
    File: tX_capabilities.cc
 
    Description: Aquire CAP_SYS_NICE through Linux' capabilities.
*/    

#include "tX_capabilities.h"
#include "tX_global.h"
#include <errno.h>
#include <string.h>

#ifdef USE_CAPABILITIES

bool have_nice_capability()
{
	cap_t caps;
	cap_flag_value_t cap;

	caps=cap_get_proc();
	
	if (!caps) {
		tX_error("have_nice_capability(): failed to get caps: %s.", strerror(errno));
		return false;
	}

	cap_get_flag(caps, CAP_SYS_NICE, CAP_EFFECTIVE, &cap);
	
	if (cap==CAP_CLEAR) {
		return false;
	}
	
	return true;
}

void set_nice_capability(cap_flag_t cap_flag) {
	cap_t caps;
	const unsigned caps_size = 1;
	cap_value_t cap_list[] = { CAP_SYS_NICE };
	
	caps=cap_get_proc();
	
	if (!caps) {
		tX_error("set_capabilities(): failed to get caps: %s.", strerror(errno));
		return;
	}
	
	cap_set_flag(caps, cap_flag, caps_size, cap_list , CAP_SET);
	
	if (cap_set_proc(caps))  {
		tX_error("set_capabilities(): failed to set caps: %s.", strerror(errno));
	}
}

#endif // USE_CAPABILITIES
