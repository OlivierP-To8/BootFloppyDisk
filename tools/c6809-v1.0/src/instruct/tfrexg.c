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
#endif

#include "defs.h"
#include "assemble.h"
#include "error.h"
#include "output.h"
#include "arg.h"
#include "output/bin.h"



/* read_register:
 *  Lit un registre.
 */
static void read_register (void)
{
    int rcode = arg_Read (&assemble.ptr, ARG_STYLE_REGISTER);

    if ((rcode & ISREG) == 0)
    {
        (void)error_BadRegister ();
    }
 
    bin.buf[2] *= '\x10';
    bin.buf[2] += (char)(rcode & 0xf);
}



/* read_comma:
 *  Lit une virgule;
 */
static void read_comma (void)
{
    if ((arg_Read (&assemble.ptr, ARG_STYLE_SIGN) != ARG_SIGN)
     || (*arg.str != ','))
    {
        (void)error_BadSeparator ();
    }
}


/* ------------------------------------------------------------------------- */


/* tfrexg_Assemble:
 *  Assemble une instruction TFR ou EXG.
 */
void tfrexg_Assemble (void)
{
    output_SetCode (OUTPUT_CODE_2_FOR_2);
    read_register ();
    read_comma ();
    read_register ();

    /* Vérifie si erreur de registres */
    if (((bin.buf[2] & '\x88') != '\x00')
     && ((bin.buf[2] & '\x88') != '\x88'))
    {
        (void)error_Error ((is_fr)?"{o}registres de taille différente"
                                  :"{o}size of registers are different");
    }
}

