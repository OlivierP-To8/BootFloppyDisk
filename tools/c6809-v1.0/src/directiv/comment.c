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
#endif

#include "defs.h"
#include "assemble.h"
#include "error.h"
#include "arg.h"
#include "output.h"

static int comment_line = 0;


/* ------------------------------------------------------------------------- */


/* comment_Assemble:
 *  Assemble la directive COMMENT.
 */
void comment_Assemble (void)
{
    assemble.lock ^= ASSEMBLE_LOCK_COMMENT;

    if ((assemble.lock & ASSEMBLE_LOCK_COMMENT) != 0)
    {
        comment_line = assemble.line;
        arg_CaseOn ();
    }
    else
    {
        arg_CaseOff ();
    }

    output_SetCode (OUTPUT_CODE_COMMENT);
}



/* comment_EndError:
 *  Génère une erreur à la fin d'un include.
 */
void comment_EndError (void)
{
    if ((assemble.lock & ASSEMBLE_LOCK_COMMENT) != 0)
    {
        (void)error_Error ((is_fr)?"{@%d}commentaire non fermé"
                                  :"{@%d}comment not closed",
                                   comment_line);
    }
}

