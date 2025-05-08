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
#include "arg.h"
#include "mark.h"
#include "output/bin.h"

static char stack_table[12] = {
    '\x06',   /* D  = 0x00 */
    '\x10',   /* X  = 0x01 */
    '\x20',   /* Y  = 0x02 */
    '\x40',   /* U  = 0x03 */
    '\x40',   /* S  = 0x04 */
    '\x80',   /* PC = 0x05 */
    '\xff',   /* ?? = 0x06 */
    '\xff',   /* ?? = 0x07 */
    '\x02',   /* A  = 0x08 */
    '\x04',   /* B  = 0x09 */
    '\x01',   /* CC = 0x0A */
    '\x08'    /* DP = 0x0B */
};



static void pushpull (int exclude)
{
    int reg;
    int last = ARG_SIGN; /* Code arbitraire différent de ARG_END */

    bin.buf[2] = '\0';
    output_SetCode (OUTPUT_CODE_2_FOR_2);

    while (last != ARG_END)
    {
        if (arg_IsEnd (*assemble.ptr) != 0)
        {
            (void)error_MissingRegister ();
        }

        switch (reg = arg_Read (&assemble.ptr, ARG_STYLE_REGISTER))
        {
            case ARG_END:
                break;

            case ARG_ALPHA:
            case ARG_NUMERIC:  (void)error_BadRegister (); break;

            case ARG_SIGN:
                if (*arg.str == ',')
                {
                    (void)error_MissingRegister ();
                }
                else
                {
                    (void)error_BadSeparator ();
                }
                break;

            default:
                if ((reg == exclude)
                 || ((bin.buf[2] & stack_table[reg&0xff]) != '\0'))
                {
                    (void)error_BadRegister ();
                }

                mark_AddPlus (((reg & 0xff) > (ARG_PC & 0xff)) ?1 :2);
                bin.buf[2] |= stack_table[reg&0xff];
                break;
        }

        switch (last = arg_Read (&assemble.ptr, ARG_STYLE_SIGN))
        {
            case ARG_END:
                break;

            case ARG_ALPHA:
            case ARG_NUMERIC: (void)error_BadRegister ();  break;

            default:
                if (*arg.str != ',')
                {
                    (void)error_BadSeparator ();
                }
                break;
        }
    }
}


/* ------------------------------------------------------------------------- */


/* pushpull_AssembleS:
 *  Assemble une instruction PSHS/PULS.
 */
void pushpull_AssembleS (void)
{
    pushpull (ARG_S);
}



/* pushpull_AssembleU:
 *  Assemble une instruction PSHU/PULU.
 */
void pushpull_AssembleU (void)
{
    pushpull (ARG_U);
}

