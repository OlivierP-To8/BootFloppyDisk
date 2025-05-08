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
#endif

#include "defs.h"
#include "assemble.h"
#include "error.h"
#include "output.h"
#include "eval.h"
#include "arg.h"
#include "directiv/end.h"

static u16 end_exec = 0;
static int end_done = 0;


/* ------------------------------------------------------------------------- */


/* end_AssembleEND:
 *  Assemble la directive END.
 */
void end_AssembleEND (void)
{
    error_LabelNotSupported ();
    end_exec = 0;
    if (arg_IsEnd (*assemble.ptr) == 0)
    {
        (void)eval_Do (&assemble.ptr, &end_exec);
    }

    end_done = 1;
    output_SetCode (OUTPUT_CODE_END);
}



/* end_IsEnd:
 *  Retourne le flag de fin de programme.
 */
int end_IsEnd (void)
{
    return end_done;
}



/* end_GetExec:
 *  Retourne l'adresse d'exécution.
 */
u16 end_GetExec (void)
{
    return end_exec;
}



/* end_Init:
 *  Initialise la directive END.
 */
void end_Init (void)
{
    end_exec = 0;
    end_done = 0;
}

