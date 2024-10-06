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
#include "arg.h"
#include "output/bin.h"


/* ------------------------------------------------------------------------- */


/* immediat_AssembleImmediate:
 *  Assemble les instructions avec adressage immédiat.
 */
void immediat_Assemble (void)
{
    int err = 0;
    int value = 0;

    output_SetCode (OUTPUT_CODE_2_FOR_2);

    if (arg_Read (&assemble.ptr, ARG_STYLE_ADDRESSING) == ARG_SIGN)
    {
        if (*arg.str == '#')
        {
            err = eval_Do (&assemble.ptr, &value);
            if ((value < -256) || (value > 255))
            {
                err = error_Error ((is_fr)?"{o}valeur 8 bits attendue"
                                          :"{o}8 bits value expected");
            }
        }
        else
        {
            err = error_Error ((is_fr)?"{+}préfixe '#' attendu"
                                      :"{+}prefix '#' expected");
        }
    }
    else
    {
        err = error_Error ((is_fr)?"{-}préfixe '#' attendu"
                                  :"{-}expect '#' prefix");
    }

    bin.buf[2] = (err == 0) ? (char)value : '\0';
}

