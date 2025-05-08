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

#ifndef C6809_ENCODE_H
#define C6809_ENCODE_H 1

enum {
    ENCODING_UNKNOWN = -1,
    ENCODING_UTF8,
    ENCODING_WINDOWS,
    ENCODING_MSDOS,
    ENCODING_MAC
};

extern void  encode_Os (int encoding);
extern int   encode_Get (int in, char *str);
extern char  /*@dependent@*/*encode_String (int in, int out, char *p);
extern char  /*@dependent@*/*encode_GetChar (int in, char **p);
extern char  /*@dependent@*/*encode_GetCharToAsm (int in, char **p);
extern char  /*@dependent@*/*encode_GetCharToAsc (int in, char **p);

#endif

