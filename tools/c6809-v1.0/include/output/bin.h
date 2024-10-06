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

#ifndef C6809_BIN_H
#define C6809_BIN_H 1

#define BIN_SIZE_MAX   16
#define BIN_SIZE_MIN   4

struct BIN_STRUCT {
    char  buf[1+BIN_SIZE_MAX];
    int   size;
    int   max;
};

extern struct BIN_STRUCT bin;

extern void  bin_Init (void);
extern void  bin_Clear (void);
extern void  bin_Reset (void);
extern int   bin_ROpen (char *name, char *extension);
extern int   bin_GetC (void);
extern void  bin_Open (char *name);
extern void  bin_PutC (char c);
extern void  bin_Flush (void);
extern void  bin_MoveOrg (void);
extern void  bin_RClose (void);
extern void  bin_WClose (void);
extern void  bin_SetBinName (char *name);
/* options */
extern void  bin_SetNonLinearFile (void);
extern void  bin_SetLinearFile (void);
extern void  bin_SetHybridFile (void);
extern void  bin_SetDataFile (void);

#endif

