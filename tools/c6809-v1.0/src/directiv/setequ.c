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
#endif

#include "defs.h"
#include "arg.h"
#include "assemble.h"
#include "error.h"
#include "output.h"
#include "eval.h"
#include "symbol.h"
#include "directiv/setequ.h"

static int setequ_value = 0;



/* assemble_setequ:
 *  Assemble les directives SET et EQU.
 */
static void assemble_setequ (int type)
{
    int err = 0;

    if (assemble.label[0] == '\0')
    {
        (void)error_Error ((is_fr)?"{Cc}étiquette obligatoire"
                                  :"{Cc}label required");
    }
    else
    {
        err = eval_Do (&assemble.ptr, &setequ_value);
        if (err == 0)
        {
            assemble_Label (type, setequ_value);
        }
        output_SetCode (OUTPUT_CODE_EQU);
    }
}


/* ------------------------------------------------------------------------- */


/* setequ_Set:
 *  Initialise la valeur du SET/EQU.
 */
void setequ_Set (int value)
{
    setequ_value = value;
}



/* setequ_Get:
 *  Retourne la valeur du SET/EQU.
 */
int setequ_Get (void)
{
    return setequ_value;
}



/* setequ_AssembleSET:
 *  Assemble la directive SET.
 */
void setequ_AssembleSET (void)
{
    assemble_setequ (SYMBOL_TYPE_SET);
}



/* setequ_AssembleEQU:
 *  Assemble la directive EQU.
 */
void setequ_AssembleEQU (void)
{
    assemble_setequ (SYMBOL_TYPE_EQU);
}

