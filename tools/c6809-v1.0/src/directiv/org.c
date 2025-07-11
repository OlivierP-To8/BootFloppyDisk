/*
 *  c6809 version 1.0.3
 *  copyright (c) 2025 Fran�ois Mouret
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
#endif

#include "defs.h"
#include "assemble.h"
#include "eval.h"
#include "error.h"
#include "output.h"
#include "assemble.h"
#include "symbol.h"
#include "directiv/org.h"

static u16 org_pc = 0;


/* ------------------------------------------------------------------------- */


/* org_AssembleORG:
 *  Ex�cute la directive ORG.
 */
void org_AssembleORG (void)
{
    error_LabelNotSupported ();
    (void)eval_Do (&assemble.ptr, &org_pc);
    output_SetCode (OUTPUT_CODE_PC);
}



/* org_Set:
 *  D�finit la valeur de ORG.
 */
void org_Set (u16 org)
{
    org_pc = org;
}



/* org_Get:
 *  Retourne la valeur de ORG.
 */
u16 org_Get (void)
{
    return org_pc;
}

