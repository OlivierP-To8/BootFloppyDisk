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

#ifndef C6809_OUTPUT_H
#define C6809_OUTPUT_H 1

enum {
    OUTPUT_CODE_NONE = 0,
    OUTPUT_CODE_COMMENT,
    OUTPUT_CODE_BYTES,
    OUTPUT_CODE_BYTES_ONLY,
    OUTPUT_CODE_WORDS,
    OUTPUT_CODE_WORDS_ONLY,
    OUTPUT_CODE_PC,
    OUTPUT_CODE_END,
    OUTPUT_CODE_EQU,
    OUTPUT_CODE_DP,
    OUTPUT_CODE_1_FOR_1,
    OUTPUT_CODE_2_FOR_2,
    OUTPUT_CODE_2_FOR_3,
    OUTPUT_CODE_3_FOR_3,
    OUTPUT_CODE_3_FOR_4
};

extern void  output_SetCode (int code);
extern void  output_PrintCode (void);
extern void  output_Print (char *str);
extern void  output_PrintSource (char *str);
extern void  output_PrintCr (void);
extern int   output_File (const char *format, ... );
extern void  output_FileSource (char *str);
extern void  output_FileCr (void);
extern void  output_SetStyle (int style);
extern int   output_GetStyle (void);

#endif

