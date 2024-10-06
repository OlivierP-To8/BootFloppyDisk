/*
 *  c6809 version 1.0.0
 *  copyright (c) 2024 François Mouret
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

#ifndef C6809_MARK_H
#define C6809_MARK_H 1

enum {
    MARK_TYPE_NONE = 0,
    MARK_TYPE_INFO,
    MARK_TYPE_CHECK,
    MARK_TYPE_MAIN,
    MARK_TYPE_INCLUDE
};

extern void  mark_Init (void);
extern int   mark_IsMark (char **p);
extern void  mark_Read (void);
extern void  mark_Clear (void);
extern void  mark_Reset (void);
extern void  mark_AddCycle (int cycles);
extern void  mark_AddPlus (int cycles);
extern void  mark_AddSize (int size);
extern int   mark_GetCycle (void);
extern int   mark_GetPlus (void);

#endif

