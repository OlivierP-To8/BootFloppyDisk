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
#include "source.h"
#include "assemble.h"
#include "encode.h"

static FILE /*@null@*/*lst_file = NULL;
static char *lst_cr;
static int  lst_encoding = ENCODING_UNKNOWN; /* Encodage pour le fichier lst */



/* print_lst:
 *  Ecrit dans le fichier.
 */
static int print_lst (const char *format, ... )
{
    va_list args;
    char str[MAX_STRING+1];
    int pos = 0;

    va_start (args, format);
    pos = vsnprintf (str, MAX_STRING, format, args);
    if (lst_file != NULL)
    {
        (void)fprintf (lst_file, "%s", str);
    }
    va_end (args);

    return pos;
}


/* ------------------------------------------------------------------------- */


/* lst_Open:
 *  Ouvre le fichier.
 */
void lst_Open (char *name)
{
    static int first = 1;

    if (lst_file == NULL)
    {
        lst_file = fopen (name, "wb");
        if ((first != 0)
         && (lst_file == NULL))
        {
            first = 0;
            (void)error_ErrnoFOpen (name);
            error_PrintForced ();
        }
    }
}



/* lst_Print:
 *  Ecrit dans le fichier.
 */
void lst_Print (int encoding, char *str)
{
    (void)print_lst ("%s", encode_String (encoding, lst_encoding, str));
}



/* lst_PrintSource:
 *  Ecrit avec l'encodage du source.
 */
void lst_PrintSource (char *str)
{
    if (assemble.source != NULL)
    {
        (void)print_lst ("%s", encode_String (assemble.source->encoding,
                                              lst_encoding, str));
    }
}



/* lst_PrintCr:
 *  Ecrit un retour de ligne.
 */
void lst_PrintCr (void)
{
    (void)print_lst ("%s", lst_cr);
}



/* lst_PrintSourceLine:
 *  Ecrit la ligne assembleur parsée avec l'encodage du source.
 */
void lst_PrintSourceLine (void)
{
    struct ARG_LIST *list;

    if (arg.list != NULL)
    {
        for (list = arg.list; list != NULL; list = list->next)
        {
            if (list->str != NULL)
            {
                lst_PrintSource (list->str);
            }
        }
    }
}



/* lst_SetCR:
 *  Définit le retour de ligne.
 */
void lst_SetCR (char *cr)
{
    lst_cr = cr;
}



/* lst_Close:
 *  Ferme le fichier.
 */
void lst_Close (void)
{
    if (lst_file != NULL)
    {
        (void)fclose (lst_file);
        lst_file = NULL;
    }
}



/* lst_GetEncoding:
 *  Retourne l'encodage.
 */
int lst_GetEncoding (void)
{
    return lst_encoding;
}



/* lst_SetEncoding:
 *  Définit l'encodage.
 */
void lst_SetEncoding (int encoding)
{
    lst_encoding = encoding;
}

