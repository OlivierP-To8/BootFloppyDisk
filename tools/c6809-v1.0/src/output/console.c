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

#ifndef SCAN_DEPEND
    #include <stdio.h>
    #include <string.h>
    #include <stdarg.h>
#endif

#include "defs.h"
#include "error.h"
#include "arg.h"
#include "output.h"
#include "source.h"
#include "assemble.h"
#include "encode.h"
#include "output/console.h"

static char con_color_none[]  = "";
static char con_color_red[]   = "\x1b[31m";
static char con_color_green[] = "\x1b[32m\x1b[2m";
static char con_color_blue[]  = "\x1b[34m";

static char con_style_none[]   = "";
static char con_style_bold[]   = "\x1b[1m";
static char con_style_invert[] = "\x1b[7m";

static char con_reset[] = "\x1b[0m";

static FILE /*@null@*/*con_file = NULL;
static int  con_encoding = ENCODING_UNKNOWN; /* Encodage pour la console */
static int  con_error = 0;
static int  con_write_door = CONSOLE_WRITE_DOOR_OPENED;
static int  con_quiet = CONSOLE_QUIET_OFF;



/* print_con:
 *  Ecrit dans la console.
 */
static int print_con (const char *format, ... )
{
    va_list args;
    char str[MAX_STRING+1];
    int pos = 0;

    va_start (args, format);
    pos = vsnprintf (str, MAX_STRING, format, args);
    if (con_file != NULL)
    {
        (void)fprintf (con_file, "%s", str);
    }
    va_end (args);

    return pos;
}



/* print_tag:
 *  Ecrit une balise.
 */
static void print_tag (char *tag)
{
#ifdef OS_LINUX
    (void)print_con ("%s", tag);
#else
    tag = tag;
#endif
}



/* open_tag:
 *  Ecrit un élément du parsing.
 */
static void open_tag (int encoding, char *str, char *color, char *style)
{
    if (color != con_color_none)
    {
        print_tag (color);
    }

    if (style != con_style_none)
    {
        print_tag (style);
    }

    if (con_error != 0)
    {
        print_tag (con_style_bold);
        print_tag (con_style_invert);
    }

    (void)print_con ("%s", encode_String (encoding, con_encoding, str));

    if ((color != con_color_none)
     || (style != con_style_none)
     || (con_error != 0))
    {
        print_tag (con_reset);
    }
}


/* ------------------------------------------------------------------------- */


/* con_Open:
 *  Ouvre la console.
 */
void con_Open (void)
{
    static int first = 1;

    if (con_file == NULL)
    {
        con_file = stderr;
        if (con_file == NULL)
        {
            con_file = stdout;
        }
        
        if ((first != 0)
         && (con_file == NULL))
        {
            first = 0;
            (void)error_ErrnoFOpen ("stdout");
            error_PrintForced ();
        }
    }

    con_quiet = CONSOLE_QUIET_OFF;
    con_write_door = CONSOLE_WRITE_DOOR_OPENED;
}



/* con_Print:
 *  Ecrit avec balise.
 */
void con_Print (int encoding, char *str)
{
    char *color;
    char *style;
    
    style = con_style_bold;

    if (con_write_door == CONSOLE_WRITE_DOOR_OPENED)
    {
        switch (output_GetStyle ())
        {
            case ARG_STYLE_BOLD:      color = con_color_none;  break;
            case ARG_STYLE_OPTIMIZE:  color = con_color_green; break;
            case ARG_STYLE_WARNING:   color = con_color_green; break;
            case ARG_STYLE_THOMSON:   color = con_color_blue;  break;
            case ARG_STYLE_ASSEMBLER: color = con_color_blue;  break;
            case ARG_STYLE_ERROR:     color = con_color_red;   break;
            case ARG_STYLE_FATAL:     color = con_color_red;   break;
            default: color = con_color_none;
                     style = con_style_none;
                     break;
        }
        open_tag (encoding, str, color, style);
    }
}



/* con_PrintCr:
 *  Ecrit un CR.
 */
void con_PrintCr (void)
{
    con_Print (ENCODING_WINDOWS, "\n");
    con_error = 0;
}



/* con_PrintSource:
 *  Ecrit avec l'encodage du source.
 */
void con_PrintSource (char *str)
{
    con_error = 0;
    if (assemble.source != NULL)
    {
        (void)con_Print (assemble.source->encoding, str);
    }
}



/* con_PrintSourceLine:
 *  Ecrit la ligne assembleur parsée.
 */
void con_PrintSourceLine (void)
{
    struct ARG_LIST *list;

    if ((assemble.source != NULL)
     && (arg.list != NULL))
    {
        con_Print (assemble.source->encoding, "        ");
        for (list = arg.list; list != NULL; list = list->next)
        {
            con_error = list->error;
            if (list->str != NULL)
            {
                con_Print (assemble.source->encoding, list->str);
            }
        }
    }
}



/* con_Close:
 *  Ferme la console.
 */
void con_Close (void)
{
}



/* con_SetQuiet:
 *  Oblitère la sortie Thomson.
 */
void con_SetQuiet (int flag)
{
    con_quiet = flag;
}



/* con_GetQuiet:
 *  Retourne l'état de la sortie Thomson.
 */
int con_GetQuiet (void)
{
    return con_quiet;
}



/* con_SetWriteDoor:
 *  Autorise la sortie console.
 */
void con_SetWriteDoor (int flag)
{
    con_write_door = flag;
}



/* con_GetWriteDoor:
 *  Retourne le flag de la sortie console.
 */
int con_GetWriteDoor (void)
{
    return con_write_door;
}



/* con_SetEncoding:
 *  Définit l'encodage de la sortie.
 */
void con_SetEncoding (int encoding)
{
    con_encoding = encoding;
}

