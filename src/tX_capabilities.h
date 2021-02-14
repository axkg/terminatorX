/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2021  Alexander König

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

    File: tX_capabilities.h

    Description: Aquire CAP_SYS_NICE through Linux' capabilities.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef USE_CAPABILITIES
#include <sys/prctl.h>

#undef _POSIX_SOURCE
#include <sys/capability.h>

extern bool have_nice_capability();
extern void set_nice_capability(cap_flag_t cap_flag);

#endif // USE_CAPABILITIES
