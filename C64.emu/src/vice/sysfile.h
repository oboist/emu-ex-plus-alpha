/*
 * sysfile.h - Simple locator for VICE system files.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#ifndef VICE_SYSFILE_H
#define VICE_SYSFILE_H

#include <stdio.h>

#include "types.h"

extern int sysfile_init(const char *emu_id);
extern void sysfile_shutdown(void);
extern int sysfile_resources_init(void);
extern void sysfile_resources_shutdown(void);
extern int sysfile_cmdline_options_init(void);
extern FILE *sysfile_open(const char *name, const char *subpath, char **complete_path_return, const char *open_mode);
extern int sysfile_locate(const char *name, const char *subpath, char **complete_path_return);
extern int sysfile_load(const char *name, const char *subpath, uint8_t *dest, int minsize, int maxsize);

#include <viceSysFileAPI.h>

#endif
