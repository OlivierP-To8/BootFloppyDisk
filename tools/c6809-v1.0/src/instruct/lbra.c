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
#include "output.h"
#include "eval.h"
#include "directiv/org.h"
#include "output/bin.h"


/* ------------------------------------------------------------------------- */


/* lbra_Assemble:
 *  Assemble une instruction de branchement long.
 */
void lbra_Assemble (void)
{
    int value = 0;

    output_SetCode (OUTPUT_CODE_2_FOR_3);

    (void)eval_Do (&assemble.ptr, &value);
    value -= org_Get() + bin.size + ((bin.buf[0] != '\0') ? 1 : 0);

    if ((value >= -0x80) && (value <= 0x7f))
    {
        (void)error_Optimize ((is_fr)?"{Co}branchement court possible"
                                     :"{Co}could be short branch");
    }

    bin.buf[2] = (char)((uint)value >> 8);
    bin.buf[3] = (char)value;
}

