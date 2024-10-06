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
    #include <stdarg.h>
#endif

#include "defs.h"
#include "arg.h"
#include "assemble.h"
#include "error.h"
#include "output.h"
#include "eval.h"
#include "output/thomson.h"

static char echo_string[MAX_STRING+1];
static int  echo_string_length = 0;



/* p_echo:
 *  Ajoute une chaîne à la sortie Thomson.
 */
static void p_echo (const char *format, ... )
{
    va_list args;

    va_start (args, format);
    echo_string_length += vsnprintf (echo_string+echo_string_length,
                                      MAX_STRING, format, args);
    va_end (args);
}



/* binary_number:
 *  Renvoie un nombre en base binaire.
 */
static char /*@dependent@*/*binary_number (int val)
{
    int i;
    int digit = 0;
    static char str[17];

    str[0] = '\0';
    for (i = 15; i >= 0; i--)
    {
        if ((digit = (int)((digit * 2) | (((uint)val >> (uint)i) & 1))) != 0)
        {
            strcat (str, ((digit & 1) == 0) ? "0" : "1");
        }
    }
    return str;
}



/* create_echo_string:
 *  Crée la chaîne pour la sortie de ECHO.
 */
static void create_echo_string (void)
{
    char c;
    int value = 0;

    echo_string[0] = '\0';
    echo_string_length = 0;

    while (*assemble.ptr != '\0')
    {
        arg_ReadChar (&assemble.ptr, ARG_STYLE_SIGN);
        if (strchr ("%@&$", *arg.str) != NULL)
        {
            c = *arg.str;
            (void)eval_Do (&assemble.ptr, &value);

            switch (c)
            {
                case '@': p_echo ("@%o", (int)value&0xffff); break;
                case '&': p_echo ("%d", (int)value&0xffff); break;
                case '$': p_echo ("$%04X", (int)value&0xffff); break;
                case '%': p_echo ("%%%s", binary_number (value)); break;
            }
        }
        else
        {
            p_echo ("%s", arg.str);
        }
    }
}



/* allow:
 *  Limite l'autorisation de l'usage.
 */
static void allow (void)
{
    error_DirectiveNotSupported ((assemble.soft < ASSEMBLER_MACRO) ? 1 : 0);
    error_LabelNotSupported ();
}


/* ------------------------------------------------------------------------- */


/* print_AssembleECHO:
 *  Assemble la directive ECHO.
 */
void print_AssembleECHO (void)
{
    allow ();
    if (assemble.pass == ASSEMBLE_PASS2)
    {
        assemble_Flush (OUTPUT_CODE_COMMENT);
        create_echo_string ();
        thm_PrintSource (echo_string);
        output_SetCode (OUTPUT_CODE_NONE);
    }
}



/* print_AssemblePRINT:
 *  Assemble la directive PRINT.
 */
void print_AssemblePRINT (void)
{
    allow ();
    if (assemble.pass == ASSEMBLE_PASS1)
    {
        assemble_Flush (OUTPUT_CODE_COMMENT);
        thm_PrintSource (assemble.ptr);
        output_SetCode (OUTPUT_CODE_NONE);
    }
}

