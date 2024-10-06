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

#ifndef SCAN_DEPEND
    #include <stdio.h>
    #include <stdarg.h>
#endif

#include "defs.h"
#include "arg.h"
#include "source.h"
#include "assemble.h"
#include "error.h"
#include "output.h"
#include "mark.h"
#include "encode.h"
#include "directiv/end.h"
#include "directiv/setequ.h"
#include "directiv/setdp.h"
#include "directiv/org.h"
#include "directiv/macro.h"
#include "output/bin.h"
#include "output/lst.h"
#include "output/html.h"
#include "output/console.h"

#if 0
#define DO_PRINT  1      /* if output wanted */
#endif

#define COLUMN_LINE_NUMBER 0
#define COLUMN_CYCLE       9
#define COLUMN_PC          15
#define COLUMN_CODE        21
#define COLUMN_DP          31
#define COLUMN_EQU         COLUMN_DP
#define COLUMN_END         COLUMN_DP
#define COLUMN_STRING      39

static int output_code = OUTPUT_CODE_COMMENT;
static int output_style = ARG_STYLE_NONE;



/* move_cursor:
 *  Déplace le curseur à la colonne demandée et détermine le style.
 */
static int move_cursor (int pos, int max, int style)
{
    output_style = ARG_STYLE_NONE;
    for (; pos < max; pos++)
    {
        (void)output_File (" ");
    }
    output_style = style;

    return pos;
}
    


/* output_line:
 *  Affiche la chaîne pour le numéro de ligne.
 */
static int output_line (int pos)
{
    switch (output_code)
    {
        case OUTPUT_CODE_BYTES_ONLY:
        case OUTPUT_CODE_WORDS_ONLY:
            break;

        default:
            pos = move_cursor (pos, COLUMN_LINE_NUMBER, ARG_STYLE_NONE);
            pos += output_File ("% 7d", assemble.line);
            break;
    }

    return pos;
}
            


/* output_pc:
 *  Affiche la chaîne pour le PC.
 */
static int output_pc (int pos)
{
    switch (output_code)
    {
        case OUTPUT_CODE_COMMENT:
        case OUTPUT_CODE_END:
        case OUTPUT_CODE_EQU:
        case OUTPUT_CODE_DP:
            break;

        default:
            pos = move_cursor (pos, COLUMN_PC, ARG_STYLE_ADDRESS);
            pos += output_File ("%04X", org_Get()&0xffff);
            break;
    }

    return pos;
}



/* output_cycle:
 *  Affiche la chaîne pour le nombre de cycles.
 */
static int output_cycle (int pos)
{
    switch (output_code)
    {
        case OUTPUT_CODE_PC:
        case OUTPUT_CODE_1_FOR_1:
        case OUTPUT_CODE_2_FOR_2:
        case OUTPUT_CODE_2_FOR_3:
        case OUTPUT_CODE_3_FOR_3:
        case OUTPUT_CODE_3_FOR_4:
            pos = move_cursor (pos, COLUMN_CYCLE, ARG_STYLE_NONE);
            pos += (mark_GetCycle()>0) ?output_File ("%d", mark_GetCycle()) :0;
            pos += (mark_GetPlus()<0) ?0 :output_File ("+%d", mark_GetPlus());
            break;
    }

    return pos;
}



/* output_opcode:
 *  Affiche la chaîne pour l'opcode.
 */
static int output_opcode (void)
{
    int pos;

    pos = (bin.buf[0] == '\0') ? output_File ("  ")
                               : output_File ("%02x", (uchar)bin.buf[0]);
    pos += output_File ("%02X", (uchar)bin.buf[1]);

    return pos;
}



/* output_binary:
 *  Affiche la chaîne pour les codes binaires.
 */
static int output_binary (int pos)
{
    int i;

    switch (output_code)
    {
        case OUTPUT_CODE_BYTES:
        case OUTPUT_CODE_BYTES_ONLY:
            pos = move_cursor (pos, COLUMN_CODE, ARG_STYLE_CODE);
            for (i = 1; i <= bin.size;)
            {
                pos += output_File ((i != 0) ? " " : "");
                pos += output_File ("%02X", (uchar)bin.buf[i++]);
            }
            break;

        case OUTPUT_CODE_WORDS:
        case OUTPUT_CODE_WORDS_ONLY:
            pos = move_cursor (pos, COLUMN_CODE, ARG_STYLE_CODE);
            for (i = 1; i <= bin.size;)
            {
                pos += output_File ((i != 0) ? " " : "");
                pos += output_File ("%02X", (uchar)bin.buf[i++]);
                pos += output_File ("%02X", (uchar)bin.buf[i++]);
            }
            break;

        case OUTPUT_CODE_END:
            pos = move_cursor (pos, COLUMN_END, ARG_STYLE_CODE);
            pos += output_File ("%04X", end_GetExec()&0xffff);
            break;

        case OUTPUT_CODE_EQU:
            pos = move_cursor (pos, COLUMN_EQU, ARG_STYLE_CODE);
            pos += output_File ("%04X", setequ_Get()&0xffff);
            break;

        case OUTPUT_CODE_DP:
            pos = move_cursor (pos, COLUMN_DP, ARG_STYLE_CODE);
            pos += output_File ("%02X", setdp_Get());
            break;

        case OUTPUT_CODE_1_FOR_1:
            pos = move_cursor (pos, COLUMN_CODE, ARG_STYLE_CODE);
            pos += output_opcode ();
            break;

        case OUTPUT_CODE_2_FOR_2:
            pos = move_cursor (pos, COLUMN_CODE, ARG_STYLE_CODE);
            pos += output_opcode ();
            pos += output_File (" %02X", (uchar)bin.buf[2]);
            break;

        case OUTPUT_CODE_2_FOR_3:
            pos = move_cursor (pos, COLUMN_CODE, ARG_STYLE_CODE);
            pos += output_opcode ();
            pos += output_File (" %02X", (uchar)bin.buf[2]);
            pos += output_File ("%02X" , (uchar)bin.buf[3]);
            break;

        case OUTPUT_CODE_3_FOR_3:
            pos = move_cursor (pos, COLUMN_CODE, ARG_STYLE_CODE);
            pos += output_opcode ();
            pos += output_File (" %02X", (uchar)bin.buf[2]);
            pos += output_File (" %02X", (uchar)bin.buf[3]);
            break;

        case OUTPUT_CODE_3_FOR_4:
            pos = move_cursor (pos, COLUMN_CODE, ARG_STYLE_CODE);
            pos += output_opcode ();
            pos += output_File (" %02X", (uchar)bin.buf[2]);
            pos += output_File (" %02X", (uchar)bin.buf[3]);
            pos += output_File ("%02X" , (uchar)bin.buf[4]);
            break;
    }

    return pos;
}



/* output_text:
 *  Affiche la chaîne pour la ligne assembleur.
 */
static void output_text (int pos)
{
    switch (output_code)
    {
        case OUTPUT_CODE_BYTES_ONLY:
        case OUTPUT_CODE_WORDS_ONLY:
            break;

        default:
            if (*assemble.buf != '\0')
            {
                (void)move_cursor (pos, COLUMN_STRING, ARG_STYLE_NONE);
                lst_PrintSourceLine ();
                htm_PrintSourceLine ();
            }
            break;
    }
}


/* ------------------------------------------------------------------------- */


/* output_PrintCode:
 *  Affiche une ligne de code.
 */
void output_PrintCode (void)
{
    int pos = 0;

#ifndef DO_PRINT
    if ((assemble.pass == ASSEMBLE_PASS2)
     && (output_code != OUTPUT_CODE_NONE)
     && (macro_GetQuiet() == 0))  /* sam */
#else
    if ((output_code != OUTPUT_CODE_NONE)
     && (macro_GetQuiet() == 0))  /* sam */
#endif
    {
        output_style = ARG_STYLE_NONE;
        pos = output_line (pos);
        if (assemble.lock == ASSEMBLE_LOCK_NONE)
        {
            pos = output_cycle (pos);
            pos = output_pc (pos);
            pos = output_binary (pos);
        }
        output_text (pos);
        output_style = ARG_STYLE_NONE;
        output_FileCr ();
    }
}



/* output_SetCode:
 *  Fixe le code d'affichage.
 */
void output_SetCode (int code)
{
    switch (code)
    {
        case OUTPUT_CODE_1_FOR_1: bin.size = 1; break;
        case OUTPUT_CODE_2_FOR_2: bin.size = 2; break;
        case OUTPUT_CODE_2_FOR_3: bin.size = 3; break;
        case OUTPUT_CODE_3_FOR_3: bin.size = 3; break;
        case OUTPUT_CODE_3_FOR_4: bin.size = 4; break;
    }

    output_code = code;
}



/* output_PrintSource:
 *  Affiche une ligne de texte avec encodage du source (lst, html et console).
 */
void output_PrintSource (char *str)
{
    lst_PrintSource (str);
    htm_PrintSource (str);
    con_PrintSource (str);
}



/* output_PrintCr:
 *  Affiche une ligne de texte (lst, html et console).
 */
void output_PrintCr (void)
{
    lst_PrintCr ();
    htm_PrintCr ();
    con_PrintCr ();
}



/* output_Print:
 *  Affiche une ligne de texte dans les fichiers (lst, html et console).
 */
void output_Print (char *str)
{
    lst_Print (ENCODING_WINDOWS, str);
    htm_Print (ENCODING_WINDOWS, str);
    con_Print (ENCODING_WINDOWS, str);
}



/* output_FileSource:
 *  Affiche une ligne de texte avec encodage du source (lst et html).
 */
void output_FileSource (char *str)
{
    lst_PrintSource (str);
    htm_PrintSource (str);
}



/* output_FileCr
 *  Affiche un retour de ligne (lst et html).
 */
void output_FileCr (void)
{
    lst_PrintCr ();
    htm_PrintCr ();
}



/* output_File:
 *  Affiche une ligne de texte dans les fichiers (lst et html).
 */
int output_File (const char *format, ... )
{
    va_list args;
    char str[MAX_STRING+1];
    int size;

    va_start (args, format);
    size = vsnprintf (str, MAX_STRING, format, args);
    lst_Print (ENCODING_WINDOWS, str);
    htm_Print (ENCODING_WINDOWS, str);
    va_end (args);

    return size;
}



/* output_SetStyle:
 *  Définit le style de sortie.
 */
void output_SetStyle (int style)
{
    output_style = style;
}



/* output_GetStyle:
 *  Retourne le style de sortie.
 */
int output_GetStyle (void)
{
    return output_style;
}

