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
    #include <stdlib.h>
    #include <windows.h>
    #include <winbase.h>
#endif

#include "defs.h"
#include "encode.h"
#include "output.h"
#include "assemble.h"
#include "option.h"
#include "output/console.h"
#include "output/lst.h"

bool is_fr = 0;
static char cr[] = "\r";


/* ------------------------------------------------------------------------- */


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
    int argc=0;
#ifndef __MINGW32__
    char *argv[16];
#else
    char **argv;
#endif

#ifdef FRENCH_LANGUAGE
    is_fr = 1;
#else
    is_fr = 0;
#endif
    con_SetEncoding (ENCODING_MSDOS);   /* Encodage pour la console */
    lst_SetEncoding (ENCODING_WINDOWS); /* Encodage pour le fichier lst */
    encode_Os (ENCODING_WINDOWS);       /* Encodage pour l'OS */
    lst_SetCR (cr);  /* Retour de ligne pour fichier lst */

#ifndef __MINGW32__	
    if (*lpCmdLine)   /* Sam */
    {
        argv[argc++]=lpCmdLine++;

        while (*lpCmdLine)
            if (*lpCmdLine == ' ')
            {
                *lpCmdLine++ = '\0';
                argv[argc++]=lpCmdLine++;
            }
            else
                lpCmdLine++;
    }
#else
	/* Sam: Windows fourni des argc/argv déjà parsés qui tient 
	   compte des guillemets et des blancs. */
	argc = __argc;
	argv = (void*)__argv;
#endif

    con_Open ();
    if (option_Do (argc, argv) == 0)
    {
        assemble_Do (option_FileAss(), option_FileBin());
    }
    con_Close ();

    return EXIT_SUCCESS;
}

