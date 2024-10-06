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
#include "error.h"
#include "output.h"
#include "eval.h"
#include "symbol.h"
#include "assemble.h"
#include "directiv/org.h"
#include "output/bin.h"

#define TYPE_CALL   0x00
#define TYPE_GOTO   0x80



/* assemble_call_and_goto:
 *  Assemble CALL et GOTO.
 */
static void assemble_call_and_goto (char b7)
{
    int value = 0;

    output_SetCode (OUTPUT_CODE_2_FOR_2);
    (void)eval_Do (&assemble.ptr, &value);
    if ((value & 0xff00) != 0x0000)
    {
        (void)error_OperandOutOfRange ();
    }
    bin.buf[1] = '\x3f';
    bin.buf[2] = (char)(value) | b7;
}



/* allow:
 *  Limite l'autorisation de l'usage.
 */
static void allow (void)
{
    error_DirectiveNotSupported ((assemble.soft == ASSEMBLER_TO) ? 1 : 0);
    assemble_Label (SYMBOL_TYPE_LABEL, org_Get());
}


/* ------------------------------------------------------------------------- */


/* mo_AssembleCALL:
 *  Assemble la directive CALL.
 */
void mo_AssembleCALL (void)
{
    allow ();
    assemble_call_and_goto ('\x00');
}



/* mo_AssembleGOTO:
 *  Assemble la directive GOTO.
 */
void mo_AssembleGOTO (void)
{
    allow ();
    assemble_call_and_goto ('\x80');
}


/* mo_AssembleSTOP:
 *  Assemble la directive STOP.
 */
void mo_AssembleSTOP (void)
{
    allow ();
    output_SetCode (OUTPUT_CODE_2_FOR_3);
    memcpy (&bin.buf[1], "\xbd\xb0\x00", 3);
}

