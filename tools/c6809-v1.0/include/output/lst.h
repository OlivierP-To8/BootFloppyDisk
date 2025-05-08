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


#ifndef C6809_LST_H
#define C6809_LST_H 1

extern void  lst_SetCR (char *cr);
extern int   lst_GetEncoding (void);
extern void  lst_SetEncoding (int encoding);

extern void  lst_Open (char *name);
extern void  lst_Print (int encoding, char *str);
extern void  lst_PrintSource (char *str);
extern void  lst_PrintSourceLine (void);
extern void  lst_PrintCr (void);
extern void  lst_Close (void);

#endif

