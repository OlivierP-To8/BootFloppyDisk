/*
 *  c6809 version 1.0.3
 *  copyright (c) 2025 François Mouret
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#ifndef C6809_CONSOLE_H
#define C6809_CONSOLE_H 1

enum {
    CONSOLE_QUIET_OFF = 0,
    CONSOLE_QUIET_ON
};

enum {
    CONSOLE_WRITE_DOOR_CLOSED = 0,
    CONSOLE_WRITE_DOOR_OPENED
};

extern void  con_SetEncoding (int encoding);
extern void  con_Open (void);
extern void  con_Print (int encoding, char *str);
extern void  con_Close (void);
extern void  con_PrintSource (char *str);
extern void  con_PrintSourceLine (void);
extern void  con_PrintCr (void);
extern void  con_SetQuiet (int quiet);
extern int   con_GetQuiet (void);
extern void  con_SetWriteDoor (int door);
extern int   con_GetWriteDoor (void);

#endif

