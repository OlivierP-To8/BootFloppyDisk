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
#include "directiv/setdp.h"

static u16 setdp_value = 0;


/* ------------------------------------------------------------------------- */


/* setdp_AssembleSETDP:
 *  Exécute la directive SETDP.
 */
void setdp_AssembleSETDP (void)
{
    error_LabelNotSupported ();
    (void)eval_Do (&assemble.ptr, &setdp_value);
    if (setdp_value > 0xff)
    {
        (void)error_Error ((is_fr)?"{C}{o}valeur hors champ"
                                  :"{C}{o}value out of range");
    }
    output_SetCode (OUTPUT_CODE_DP);
}



/* setdp_Get:
 *  Retourne la valeur du DP courant.
 */
u16 setdp_Get (void)
{
    return setdp_value & 0xff;
}



/* setdp_Set:
 *  Initialise la valeur du DP courant.
 */
void setdp_Set (u16 value)
{
    setdp_value = value;
}

