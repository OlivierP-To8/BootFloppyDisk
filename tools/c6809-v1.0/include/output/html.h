/*
 *  c6809 version 1.0.3
 *  copyright (c) 2025 Fran�ois Mouret
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


#ifndef C6809_HTML_H
#define C6809_HTML_H 1

extern void htm_Open (char *name, char *source_name);
extern void htm_Print (int encoding, char *str);
extern void htm_PrintSource (char *str);
extern void htm_PrintSourceLine (void);
extern void htm_PrintCr (void);
extern void htm_Close (void);

#endif

