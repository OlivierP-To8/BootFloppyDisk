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
#include "eval.h"
#include "output.h"
#include "directiv/org.h"
#include "output/bin.h"


/* ------------------------------------------------------------------------- */


/* bra_Assemble:
 *  Assemble les instructions de branchements courts.
 */
void bra_Assemble (void)
{
    int value = 0;

    output_SetCode (OUTPUT_CODE_2_FOR_2);
    bin.buf[2] = '\xfe';

    if (eval_Do (&assemble.ptr, &value) == 0)
    {
        value -= org_Get() + 2;
        if ((value < -0x80) || (value > 0x7f))
        {
            (void)error_Error ((is_fr)?"{Co}branchement hors champ"
                                      :"{Co}branch out of range");
        }
        else
        {
            bin.buf[2] = (char)value;
        }
    }
}

