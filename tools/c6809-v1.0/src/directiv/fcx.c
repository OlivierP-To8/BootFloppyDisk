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
    #include <stdlib.h>
    #include <string.h>
#endif

#include "defs.h"
#include "arg.h"
#include "assemble.h"
#include "error.h"
#include "output.h"
#include "eval.h"
#include "symbol.h"
#include "source.h"
#include "encode.h"
#include "directiv/org.h"
#include "output/bin.h"

#define FCX_BUF_SIZE_MAX   256

struct FCX_LIST {
    int   i;
    char  /*@only@*//*@null@*/*buf;
    struct FCX_LIST /*@only@*//*@null@*/*next;
};

static int style = OUTPUT_CODE_BYTES;
static struct FCX_LIST /*@only@*//*@null@*/*fcx_list = NULL;
static int fcx_encoding = ENCODING_UTF8;



/* error_missing_closing_character:
 *  Helper pour erreur.
 */
static void error_missing_closing_character (void)
{
    (void)error_Error ((is_fr) ? "{-}caractère de clôture manquant"
                               : "{-}missing closing character");
}



/* open_list:
 *  Ajoute un buffer d'écriture.
 */
static void open_list (void)
{
    struct FCX_LIST *fcx_list_old;
    char *fcx_list_buf;

    fcx_list_old = fcx_list;
    fcx_list = malloc (sizeof (struct FCX_LIST));
    if (fcx_list != NULL)
    {
        fcx_list->next = fcx_list_old;
        fcx_list->i = 0;
        fcx_list_buf = malloc (FCX_BUF_SIZE_MAX);
        if (fcx_list_buf == NULL)
        {
            (void)error_Memory (__FILE__, __LINE__, __func__);
        }
        else
        {
            memset (fcx_list_buf, 0, FCX_BUF_SIZE_MAX);
        }

        fcx_list->buf = fcx_list_buf;
    }
    else
    {
        fcx_list = fcx_list_old;
        (void)error_Memory (__FILE__, __LINE__, __func__);
    }
}



/* write_fcx:
 *  Enregistre un caractère.
 */
static void write_fcx (char c)
{
    if ((fcx_list == NULL)
     || (fcx_list->i == FCX_BUF_SIZE_MAX))
    {
        open_list ();
    }

    if ((fcx_list != NULL)
     && (fcx_list->buf != NULL)
     && (fcx_list->i < FCX_BUF_SIZE_MAX))
    {
        fcx_list->buf[fcx_list->i++] = c;
    }
}



/* swap_list:
 *  Restaure la liste dans l'ordre d'entrée.
 */
static struct FCX_LIST /*@null@*/*swap_list (
        struct FCX_LIST /*@returned@*//*@null@*/*old_list)
{
    struct FCX_LIST /*@null@*/*list = old_list;
    struct FCX_LIST /*@null@*/*next = NULL;
    struct FCX_LIST /*@null@*/*new_next = NULL;

    while (list != NULL)
    {
        next = list->next;
        list->next = new_next;
        new_next = list;
        list = next;
    }

    return new_next;
}



/* close_list:
 *  Referme les ressources des fcc/fcb/fdb.
 */
static void close_list (void)
{
    struct FCX_LIST *list = fcx_list;
    struct FCX_LIST *next;
    
    while (list != NULL)
    {
        next = list->next;
        if (list->buf != NULL)
        {
            free (list->buf);
        }
        free (list);
        list = next;
    }
    fcx_list = NULL;
}



/* flush_fcx:
 *  Envoie les données à l'affichage.
 */
static void flush_fcx (void)
{
    int i;
    struct FCX_LIST *list;

    fcx_list = swap_list (fcx_list);

    for (list = fcx_list; list != NULL; list = list->next)
    {
        for (i = 0; i < list->i; i++)
        {
            if (bin.size == bin.max)
            {
                assemble_Flush (style);
                style = (style == OUTPUT_CODE_BYTES) ? OUTPUT_CODE_BYTES_ONLY :
                        (style == OUTPUT_CODE_WORDS) ? OUTPUT_CODE_WORDS_ONLY :
                         style;
            }

            if (list->buf != NULL)
            {
                bin_PutC (list->buf[i]);
            }
        }
    }

    assemble_Flush (style);
    output_SetCode (OUTPUT_CODE_NONE);
    close_list ();
}



/* get_char:
 *  Lit un caractère.
 */
static char /*@dependent@*/*get_char (void)
{
    char /*@dependent@*/*c;
    char /*@dependent@*/*p;

    arg_ReadChar (&assemble.ptr, ARG_STYLE_VALUE);
    p = arg.str;
    c = encode_GetCharToAsc (fcx_encoding, &p);
    *c = (*c == '\x7f') ? ' ' : *c;

    return c;
}



/* write_fcc:
 *  Assemble les FCC.
 */
static void write_fcc (void)
{
    char /*@dependent@*/*c;
    char eol;

    arg_CaseOn ();
    assemble_Label (SYMBOL_TYPE_LABEL, org_Get());
    fcx_encoding = (assemble.source != NULL) ? (assemble.source->encoding)
                                             : ENCODING_UTF8;
    style = OUTPUT_CODE_BYTES;

    if ((*assemble.ptr > ' ') && (*assemble.ptr < '\x7f'))
    {
        c = get_char ();
        eol = c[0];
        if ((*assemble.ptr < '\0') || (*assemble.ptr >= ' '))
        {
            c = get_char ();
            while ((c[0] != eol) && (c[0] != '\0') && (eol > ' '))
            {
                if (c[0] == '\x16')
                {
                    if (assemble.soft < ASSEMBLER_MACRO)
                    {
                        error_Assembler ((is_fr)
                            ?"{+}caractères étendus non supportés"
                            :"{+}extended characters not supported");
                    }
                    write_fcx (c[0]);
                    write_fcx (c[1]);
                    write_fcx (c[2]);
                }
                else
                if (c[0] >= ' ')
                {
                    write_fcx (c[0]);
                }
                else
                {
                    error_Warning (
                       (is_fr)?"{+}caractère non supporté"
                              :"{+}character not supported");
                }

                c = get_char ();
            }

            if (c[0] == '\0')
            {
                error_missing_closing_character ();
            }
        }
        else
        {
            error_missing_closing_character ();
        }
    }
    else
    {
        (void)error_MissingOperand ();
    }

    arg_CaseOff ();
}



/* write_fcb:
 *  Assemble les FCB.
 */
static void write_fxb (void)
{
    int code = ARG_ALPHA;  /* Valeur arbitraire != ARG_END */
    int value = 0;

    assemble_Label (SYMBOL_TYPE_LABEL, org_Get());

    if (arg_IsEnd (*assemble.ptr) == 0)
    {
        while (code != ARG_END)
        {
            (void)eval_Do (&assemble.ptr, &value);
            switch (style)
            {
                case OUTPUT_CODE_BYTES:
                case OUTPUT_CODE_BYTES_ONLY:
                    if ((value < -256) || (value > 255))
                    {
                        (void)error_OperandOutOfRange ();
                    }
                    write_fcx ((char)value);
                    break;

                case OUTPUT_CODE_WORDS:
                case OUTPUT_CODE_WORDS_ONLY:
                    write_fcx ((char)((uint)value >> 8));
                    write_fcx ((char)value);
                    break;
            }

            code = arg_Read (&assemble.ptr, ARG_STYLE_NONE);
            switch (code)
            {
                case ARG_END: break;
                case ARG_SIGN:
                    arg_SetStyle (ARG_STYLE_SEPARATOR);
                    if (*arg.str != ',')
                    {
                        (void)error_BadSeparator ();
                        code = ARG_END;
                    }
                    break;

                default:
                    if (arg_IsEnd (*assemble.ptr) == 0)
                    {
                        (void)error_Error ((is_fr)
                            ?"{+}fin d'opérande attendue"
                            :"{+}end of operand expected");
                        code = ARG_END;
                    }
                    break;
            }
        }
    }
    else
    {
        (void)error_MissingOperand ();
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


/* fcx_AssembleFCB:
 *  Assemble la directive FCB.
 */
void fcx_AssembleFCB (void)
{
    style = OUTPUT_CODE_BYTES;
    write_fxb ();
    flush_fcx ();
}



/* fcx_AssembleFCC:
 *  Assemble la directive FCC.
 */
void fcx_AssembleFCC (void)
{
    write_fcc ();
    flush_fcx ();
}



/* fcx_AssembleFCS:
 *  Assemble la directive FCS.
 */
void fcx_AssembleFCS (void)
{
    allow ();
    write_fcc ();
    write_fcx ('\x00');   /* Ajoute le 0 de fin */
    flush_fcx ();
}



/* fcx_AssembleFCN:
 *  Assemble la directive FCN.
 */
void fcx_AssembleFCN (void)
{
    allow ();
    write_fcc ();
    if ((fcx_list != NULL)
     && (fcx_list->buf != NULL))
    {
        fcx_list->buf[fcx_list->i-1] |= '\x80';  /* Augmente le caractère */
    }
    flush_fcx ();
}



/* fcx_AssembleFDB:
 *  Assemble la directive FDB.
 */
void fcx_AssembleFDB (void)
{
    style = OUTPUT_CODE_WORDS;
    write_fxb ();
    flush_fcx ();
}

