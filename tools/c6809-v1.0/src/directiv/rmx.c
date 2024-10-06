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
#include "symbol.h"
#include "arg.h"
#include "directiv/org.h"
#include "output/bin.h"

static int rmx_size = 0;



/* write_rmx:
 *  Enregistre un caractère de remplissage.
 */
static void write_rmx (char c, int style)
{
    if (bin.size == bin.max)
    {
        assemble_Flush (style);
    }
    bin_PutC (c);
}



/* assemble_rmx_fill:
 *  Assemble la directive RMB/RMD avec remplissage.
 */
static void assemble_rmx_fill (int count, int style)
{
    int i;
    int value = 0;

    (void)eval_Do (&assemble.ptr, &value);

    if (assemble.soft < ASSEMBLER_MACRO)
    {
        error_Assembler ((is_fr)?"{Co}valeur de remplissage non supportée"
                                :"{Co}filling value not supported");
    }

    if ((style == OUTPUT_CODE_BYTES_ONLY)
     && ((value < -256) || ((value > 255))))
    {
        (void)error_Error ((is_fr)?"{Co}valeur de remplissage hors champ"
                                  :"{Co}filling value out of range");
    }

    assemble_Flush (OUTPUT_CODE_COMMENT);

    for (i = 0; i != (count & 0xffff); i = (i + 1) & 0xffff)
    {
        if (style == OUTPUT_CODE_WORDS_ONLY)
        {
            write_rmx ((char)((uint)value >> 8), style);
        }
        write_rmx ((char)value, style);
    }

    assemble_Flush (style);
    output_SetCode (OUTPUT_CODE_NONE);
}



/* assemble_rmx_no_fill:
 *  Assemble la directive RMB/RMD sans remplissage.
 */
static void assemble_rmx_no_fill (int count, int style)
{
    output_SetCode (OUTPUT_CODE_PC);
    rmx_size = (style == OUTPUT_CODE_BYTES_ONLY) ? count : 2*count;
}



/* assemble_rmx:
 *  Assemble la directive RMB/RMD.
 */
static void assemble_rmx (int style)
{
    int code;
    int count = 0;

    bin.max = BIN_SIZE_MAX;
    assemble_Label (SYMBOL_TYPE_LABEL, org_Get());
    (void)eval_Do (&assemble.ptr, &count);
    code = arg_Read (&assemble.ptr, ARG_STYLE_SIGN);
    if (code == ARG_END)
    {
        assemble_rmx_no_fill (count, style);
    }
    else
    if (*arg.str == ',')
    {
        assemble_rmx_fill (count, style);
    }
    else
    {
        (void)error_BadSeparator ();
    }
}



/* allow:
 *  Limite l'autorisation de l'usage.
 */
static void allow (void)
{
    error_DirectiveNotSupported ((assemble.soft < ASSEMBLER_MACRO) ? 1 : 0);
}


/* ------------------------------------------------------------------------- */


/* rmx_AssembleRMB:
 *  Assemble la directive RMB.
 */
void rmx_AssembleRMB (void)
{
    assemble_rmx (OUTPUT_CODE_BYTES_ONLY);
}



/* rmx_AssembleRMD:
 *  Assemble la directive RMD.
 */
void rmx_AssembleRMD (void)
{
    allow ();
    assemble_rmx (OUTPUT_CODE_WORDS_ONLY);
}



/* rmx_GetSize:
 *  Retourne la taille du RMB/RMD.
 */
int rmx_GetSize (void)
{
    return rmx_size;
}



/* rmx_SetSize:
 *  Définit la taille du RMB/RMD.
 */
void rmx_SetSize (int size)
{
    rmx_size = size;
}

