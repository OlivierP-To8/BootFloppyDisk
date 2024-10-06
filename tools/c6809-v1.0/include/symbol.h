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

#ifndef C6809_SYMBOL_H
#define C6809_SYMBOL_H 1

enum {
    SYMBOL_TYPE_NONE = 0,
    SYMBOL_TYPE_ARG,
    SYMBOL_TYPE_SET,
    SYMBOL_TYPE_EQU,
    SYMBOL_TYPE_LABEL,
    SYMBOL_TYPE_MACRO
};

enum {
    SYMBOL_ERROR_NONE = 0,
    SYMBOL_ERROR_NOT_DEFINED,
    SYMBOL_ERROR_MULTIPLY_DEFINED,
    SYMBOL_ERROR_LONE
};


struct SYMBOL_PRM {
    char  /*@only@*//*@null@*/*name;
    int   line;
    int   value;
    int   type;
    int   changed;
    int   quiet;
    int   error;
    int   pass;
    int   times;
};

struct SYMBOL_LIST {
    struct SYMBOL_PRM /*@only@*//*@null@*/*prm;
    struct SYMBOL_LIST /*@only@*//*@null@*/*next;
};

extern struct SYMBOL_LIST /*@dependent@*//*@null@*/*symbol_Do (
                            char *name, int value, int type);
extern int   symbol_Undefined (void);
extern void  symbol_Clear (void);
extern int   symbol_GetLine (int type, char *name);
extern void  symbol_DisplayList (void);
extern void  symbol_SetNone (void);
extern void  symbol_SetAlphaOrder (void);
extern void  symbol_SetErrorOrder (void);
extern void  symbol_SetTypeOrder (void);
extern void  symbol_SetTimesOrder (void);
extern void  symbol_SetLone (void);
extern void  symbol_Close (void);

#endif

