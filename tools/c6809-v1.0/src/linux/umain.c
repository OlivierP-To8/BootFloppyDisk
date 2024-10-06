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
    #include <string.h>
    #include <stdlib.h>
    #include <locale.h>
#endif

#include "defs.h"
#include "encode.h"
#include "output.h"
#include "assemble.h"
#include "option.h"
#include "output/console.h"
#include "output/lst.h"

bool is_fr = 0;
static char cr[] = "\n";


/* ------------------------------------------------------------------------- */


int main (int argc, char *argv[])
{
    char *lang;

    lang=getenv ("LANG");
    if (lang == NULL) lang = "fr_FR";
    (void)setlocale (LC_ALL, "");
    is_fr = (strncmp (lang, "fr", 2) == 0) ? true : false;

    con_SetEncoding (ENCODING_UTF8);   /* Encodage pour la console */
    lst_SetEncoding (ENCODING_UTF8);   /* Encodage pour le fichier lst */
    encode_Os (ENCODING_WINDOWS);      /* Encodage pour l'OS */
    lst_SetCR (cr);  /* Retour de ligne pour fichier lst */

    con_Open ();
    if (option_Do (argc, argv) == 0)
    {
        assemble_Do (option_FileAss(), option_FileBin());
    }
    con_Close ();

    return EXIT_SUCCESS;
}

