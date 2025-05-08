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
    #include <stdlib.h>
    #include <string.h>
    #include <ctype.h>
#endif

#include "defs.h"
#include "error.h"
#include "arg.h"
#include "symbol.h"
#include "assemble.h"
#include "output.h"
#include "directiv/org.h"

#define PRECEDENCE_PARENTHESIS  1000
#define PRECEDENCE_END          2000

enum {
    OPERATOR_I_C6809 = 0,
    OPERATOR_I_MACROASSEMBLER,
    OPERATOR_I_ASSEMBLER,
    OPERATOR_I_ALPHA
};

struct EVAL_LIST {
    int    sign;
    int    precedence;
    u16    value;
    struct EVAL_LIST /*@only@*//*@null@*/*next;
};

struct SIGN_PRM {
    int   precedence;
    char  name[6];
};

struct SIGN_LIST {
    int  sign;
    struct SIGN_PRM prm[3];
};

enum {
    OPER_MULT = 1,
    OPER_DIV,
    OPER_MOD,
    OPER_MINUS,
    OPER_PLUS,
    OPER_LSHIFT,
    OPER_RSHIFT,
    OPER_LE,
    OPER_EQUAL,
    OPER_GE,
    OPER_GT,
    OPER_LESS,
    OPER_AND2,
    OPER_OR2,
    OPER_OR,
    OPER_NE,
    OPER_AND,
    OPER_XOR,
    SIGN_MINUS,
    SIGN_PLUS,
    SIGN_NOT,
    SIGN_COM
};

static struct SIGN_LIST operator_list[] = {
    { OPER_MULT   , { {  3, "*"     }, { 3, "*"     }, { 3, "*"     } } },
    { OPER_DIV    , { {  3, "/"     }, { 3, "/"     }, { 3, "/"     } } },
    { OPER_DIV    , { {  3, ".DIV." }, { 3, "/"     }, { 3, ".DIV." } } },
    { OPER_MOD    , { {  3, "%"     }, { 3, "%"     }, { 0, ""      } } },
    { OPER_MOD    , { {  3, ".MOD." }, { 0, ""      }, { 0, ".MOD." } } },
    { OPER_MINUS  , { {  6, "-"     }, { 6, "-"     }, { 6, "-"     } } },
    { OPER_PLUS   , { {  6, "+"     }, { 6, "+"     }, { 6, "+"     } } },
    { OPER_LSHIFT , { {  7, "<<"    }, { 3, "<"     }, { 3, "<"     } } },
    { OPER_RSHIFT , { {  7, ">>"    }, { 3, "<"     }, { 3, "<"     } } },
    { OPER_LE     , { {  8, "<="    }, { 0, ""      }, { 0, ""      } } },
    { OPER_EQUAL  , { {  9, "=="    }, { 9, "="     }, { 9, "="     } } },
    { OPER_EQUAL  , { {  9, ".EQU." }, { 0, ""      }, { 9, ".EQU." } } },
    { OPER_GE     , { {  8, "=>"    }, { 0, ""      }, { 0, ""      } } },
    { OPER_GT     , { {  8, ">"     }, { 0, ""      }, { 0, ""      } } },
    { OPER_LESS   , { {  6, "<"     }, { 0, ""      }, { 0, ""      } } },
    { OPER_AND2   , { { 13, "&&"    }, { 0, ""      }, { 0, ""      } } },
    { OPER_OR2    , { { 14, "||"    }, { 0, ""      }, { 0, ""      } } },
    { OPER_OR     , { { 12, "|"     }, { 5, "!"     }, { 5, "!"     } } },
    { OPER_OR     , { { 12, ".OR."  }, { 0, ""      }, { 5, ".OR."  } } },
    { OPER_NE     , { {  9, "!="    }, { 0, ""      }, { 0, ""      } } },
    { OPER_NE     , { {  9, ".NEQ." }, { 0, ""      }, { 0, ".NEQ." } } },
    { OPER_AND    , { { 10, "&"     }, { 4, "&"     }, { 4, "&"     } } },
    { OPER_AND    , { { 10, ".AND." }, { 0, ""      }, { 4, ".AND." } } },
    { OPER_XOR    , { { 11, "^"     }, { 5, "^"     }, { 5, "^"     } } },
    { OPER_XOR    , { { 11, ".XOR." }, { 0, ""      }, { 5, ".XOR." } } },
    { 0           , { {  0, ""      }, { 0, ""      }, { 0, ""      } } }
};

static struct SIGN_LIST sign_list[] = {
    { SIGN_MINUS  , { {  2, "-"     }, { 2, "-"     }, { 2, "-"     } } },
    { SIGN_PLUS   , { {  2, "+"     }, { 2, "+"     }, { 2, "+"     } } },
    { SIGN_NOT    , { {  2, "!"     }, { 2, ":"     }, { 0, ""      } } },
    { SIGN_NOT    , { {  2, ".NOT." }, { 0, ""      }, { 0, ".NOT." } } },
    { SIGN_COM    , { {  2, "~"     }, { 0, ""      }, { 0, ""      } } },
    { 0           , { {  0, ""      }, { 0, ""      }, { 0, ""      } } }
};

static int isoft[4] = {
    OPERATOR_I_ASSEMBLER,      /* SOFT_ASSEMBLER_TO */
    OPERATOR_I_ASSEMBLER,      /* SOFT_ASSEMBLER_MO */
    OPERATOR_I_MACROASSEMBLER, /* SOFT_MACROASSEMBLER */
    OPERATOR_I_C6809           /* SOFT_C6809 */
};

static u16 eval_value = 0;
static int eval_sign = 0;
static int eval_operator = 0;
static int eval_precedence = PRECEDENCE_END;
static int eval_soft = 0;
static struct EVAL_LIST /*@only@*//*@null@*/*eval_list = NULL;
static char *peval;



/* error_missing_information:
 *  Helper pour erreur.
 */
static int error_missing_information (void)
{
    return error_Error ((is_fr)?"{-}information manquante"
                               :"{-}missing information");
}


/* error_binary_not_supported:
 *  Helper pour erreur.
 */
static void error_binary_not_supported (int base)
{
    if ((base == 2) && (assemble.soft < ASSEMBLER_MACRO))
    {
        error_Assembler ((is_fr)?"{+}nombre binaire non supporté"
                                :"{+}binary number not supported");
    }
}



/* error_wrong_digit:
 *  Helper pour erreur.
 */
static int error_wrong_digit (void)
{
    return error_Error ((is_fr) ? "{+}chiffre incorrect" : "{+}bad digit");
}



/* error_bases_incompatible:
 *  Helper pour erreur.
 */
static int error_bases_incompatible (void)
{
    return error_Error ((is_fr) ? "{+}bases incompatibles"
                                : "{+}incompatible bases");
}



/* push_eval:
 *  Ajoute un élément d'évaluation.
 */
static int push_eval (int sign, int precedence, u16 value)
{
    int err = 0;
    struct EVAL_LIST *eval_list_old = eval_list;

    eval_list = malloc (sizeof(struct EVAL_LIST));
    if (eval_list != NULL)
    {
        eval_list->next = eval_list_old;
        eval_list->sign = sign;
        eval_list->precedence = precedence;
        eval_list->value = value;
    }
    else
    {
        eval_list = eval_list_old;
        err = error_Memory (__FILE__, __LINE__, __func__);
    }
    return err;
}



/* pull_eval:
 *  Libère un élément d'évaluation.
 */
static void pull_eval (void)
{
    struct EVAL_LIST *eval_next;

    if (eval_list != NULL)
    {
        eval_next = eval_list->next;
        free (eval_list);
        eval_list = eval_next;
    }
}



/* is_eval_end:
 *  Vérifie si fin d'argument pour l'évaluation.
 */
static int is_eval_end (char c)
{
    return ((arg_IsEnd (c) != 0) || (c == ',') || (c == ']')) ? 1 : 0;
}



/* is_alnum:
 *  Vérifie que le caractère est alphanumérique.
 */
int is_alnum (char c)
{
    return (((c >= 'a') && (c <= 'z'))
         || ((c >= 'A') && (c <= 'Z'))
         || ((c >= '0') && (c <= '9'))) ? 1 : 0;
}



/* read_numeric_value:
 *  Lit une valeur numérique.
 */
static int read_numeric_value (int base0)
{
    int  err = 0;
    int  i;
    char c1 = '\0';
    int  c;
    int  base1 = 0;
    int  pos0;
    int  pos1;

    /* Passe le nombre */
    for (i = 0, pos0 = 0; is_alnum (peval[i]) != 0; i++) {}
    if (i == 0)
    {
        err = error_missing_information ();
    }

    /* Lit le code suffixe */
    base1 = 0;
    if ((i > pos0)
     && (is_eval_end (peval[i-1]) == 0))
    {
        switch (arg_ToUpper (peval[i-1]))
        {
            case 'U': c1 = peval[--i]; base1 = 2;  break;
            case 'Q': c1 = peval[--i]; base1 = 8;  break;
            case 'O': c1 = peval[--i]; base1 = 8;  break;
            case 'T': c1 = peval[--i]; base1 = 10; break;
            case 'H': c1 = peval[--i]; base1 = 16; break;
            case 'D': if (base0 != 16) { c1 = peval[--i]; base1 = 10; } break;
        }
    }

    pos1 = (i > pos0) ? i : pos0;
    base0 = ((base0 + base1) == 0) ? 10 : base0;
    base1 = (base1 == 0) ? base0 : base1;
    base0 = (base0 == 0) ? base1 : base0;

    /* Calcule le nombre */
    for (i = pos0; i < pos1; i++)
    {
        arg_ReadChar (&peval, ARG_STYLE_VALUE);
        c = (int)arg_ToUpper (*arg.str);
        if (arg_IsXDigit ((char)c) != 0)
        {
            c -= (c < (int)'A') ? (int)'0' : (int)'A' - 10;
            err |= ((c < 0) || (c >= base0)) ? error_wrong_digit () : 0;
            eval_value = eval_value * (u16)base0 + (u16)c;
        }
        else
        {
            err |= error_BadCharacter ();
        }
    }

    /* Les préfixe et suffixe sont valides si au moins
     * l'un d'entre eux est défini ou s'ils ont le même effet */
    if (c1 != '\0')
    {
        arg_ReadChar (&peval, ARG_STYLE_VALUE);
        error_binary_not_supported (base1);
        err |= (base0 != base1) ? error_bases_incompatible () : 0;
    }

    return err;
}



/* read_numeric_value_with_base:
 *  Lit une valeur numérique avec base prédéfinie.
 */
static int read_numeric_value_with_base (void)
{
    int base = 0;

    switch (*peval)
    {
        case '@': base = 8;  arg_ReadChar (&peval, ARG_STYLE_VALUE); break;
        case '$': base = 16; arg_ReadChar (&peval, ARG_STYLE_VALUE); break;
        case '%': base = 2;  arg_ReadChar (&peval, ARG_STYLE_VALUE); break;
        case '&': base = 10; arg_ReadChar (&peval, ARG_STYLE_VALUE); break;
    }
                  
    error_binary_not_supported (base);
    return read_numeric_value (base);
}

                    
                    
/* read_pc_value:
 *  Lit la valeur du PC.
 */
static void read_pc_value (void)
{
    (void)arg_ReadChar (&peval, ARG_STYLE_SIGN);
    eval_value = org_Get();
}



/* read_alpha_value:
 *  Lit la valeur d'un caractère.
 */
static int read_alpha_value (void)
{
    int err = 0;
    char c;
    int count = 0;

    arg_CaseOn ();
    while (*peval == '\'')
    {
        arg_ReadChar (&peval, ARG_STYLE_NONE);
        c = '\0';
        switch (++count)
        {
            case 2: if (assemble.soft < ASSEMBLER_MACRO)
                    {
                        error_Assembler ((is_fr)
                            ? "{+}caractère double non supporté"
                            : "{+}double char not supported");
                    }
                    break;

            case 3: err |= error_Error ((is_fr)
                         ? "{+}caractère multiple non supporté"
                         : "{+}multiple char not supported");
                    break;
        }

        if (arg_IsEnd (*peval) != 0)
        {
            c = '\xd';
        }
        else
        {
            arg_ReadChar (&peval, ARG_STYLE_VALUE);
            c = *arg.str;
            err |= (c < '\0') ? error_BadCharacter () : 0;
        }

        eval_value = (eval_value * 256) | ((u16)c & 0xff);
    }

    arg_CaseOff ();

    return err;
}



/* read_symbol_value:
 *  Lit la valeur d'un symbole.
 */
static int read_symbol_value (void)
{
    int err = -1;
    struct SYMBOL_LIST *symbol;

    /* Lit la valeur du symbole */
    symbol = symbol_Do (arg.str, 0, SYMBOL_TYPE_NONE);
    if ((symbol != NULL)
     && (symbol->prm != NULL))
    {
        arg.value = symbol->prm->value;
        arg_SetStyle (
            (symbol->prm->type == SYMBOL_TYPE_LABEL) ? ARG_STYLE_LABEL_HREF
          : (symbol->prm->type == SYMBOL_TYPE_EQU) ? ARG_STYLE_EQU_HREF
          : (symbol->prm->type == SYMBOL_TYPE_SET) ? ARG_STYLE_SET_HREF
          : (symbol->prm->type == SYMBOL_TYPE_MACRO) ? ARG_STYLE_MACRO_HREF
          : ARG_STYLE_NONE);
        eval_value = symbol->prm->value;
        err = (symbol->prm->type == SYMBOL_TYPE_MACRO)
                ? error_Error ((is_fr)?"{+}le symbole est une macro"
                                      :"{+}the symbol is a macro") :
              (symbol->prm->error == SYMBOL_ERROR_NOT_DEFINED)
                ? error_Error ((is_fr)?"{+}symbole inconnu"
                                      :"{+}unknown symbol") :
              (symbol->prm->error == SYMBOL_ERROR_MULTIPLY_DEFINED)
                ? error_Error ((is_fr)?"{+}définition multiple"
                                      :"{+}multiple definition") : 0;
    }
    return err;
}



/* read_value:
 *  Lit une valeur.
 */
static int read_value (void)
{
    int err = 0;
    int base = 0;

    eval_value = 0;

    switch (*peval)
    {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':   err = read_numeric_value (base); break;

        case '&':
        case '@':
        case '%':
        case '$':   err = read_numeric_value_with_base (); break;

        case '.':
        case '*':   read_pc_value (); break;
        
        case '\'':  err = read_alpha_value (); break;

        default:    if (is_eval_end (*peval) != 0)
                    {
                        err = error_missing_information ();
                    }
                    else
                    switch (arg_Read (&peval, ARG_STYLE_NONE))
                    {
                        case ARG_ALPHA: err = read_symbol_value (); break;

                        case ARG_SIGN:  arg_SetStyle (ARG_STYLE_SIGN);
                                        (void)error_Error ((is_fr)
                                            ? "{+}erreur de signe"
                                            : "{+}sign error");
                                        break;

                        case ARG_END:   (void)error_Error ((is_fr)
                                            ? "{-}erreur de valeur"
                                            : "{-}value error");
                                        break;

                        default:        arg_SetStyle (ARG_STYLE_REGISTER);
                                        (void)error_Error ((is_fr)
                                            ? "{+}erreur de valeur"
                                            : "{+}value error");
                                        break;
                    }
                    break;
    }
    return err;
}



/* find_sign:
 *  Renvoie l'index du signe.
 */
static int find_sign (struct SIGN_LIST *list, int style)
{
    int i;
    int index = -1;
    char *name;
    char *upper_name = arg_Upper (peval);
    int soft = (eval_soft == 0) ? isoft[assemble.soft] : OPERATOR_I_C6809;

    for (i = 0; (list[i].sign != 0) && (index == -1); i++)
    {
        name = list[i].prm[soft].name;
        if (*name != '\0')
        {
            index = (strncmp (upper_name, name, strlen(name)) == 0) ? i : -1;
        }
    }

    if (index >= 0)
    {
        eval_precedence = list[index].prm[soft].precedence;
        eval_sign = list[index].sign;
        (void)arg_ReadSized (&peval,
                             (int)strlen (list[index].prm[soft].name),
                             style);
    }

    return index;
}
    


/* read_operator:
 *  Lit un opérateur.
 */
static int read_operator (void)
{
    int err = 0;
    int i;
    int size;
    char *p;

    if (is_eval_end (*peval) != 0)
    {
        err = error_Error ((is_fr)?"{-}opérateur manquant"
                                  :"{-}missing operator");
    }
    else
    {
        i = find_sign (operator_list, ARG_STYLE_NONE);
        eval_operator = eval_sign;
        if (i < 0)
        {
            size = ((*peval == '.')
                && ((p = strchr (peval+1, (int)'.')) != NULL))
                && ((p - peval) <= 5) ? (p - peval) : 1;

            eval_operator = OPER_PLUS;
            eval_precedence = 1;
            (void)arg_ReadSized (&peval, size, ARG_STYLE_NONE);
            err = error_Error ((is_fr)?"{+}opérateur inconnu"
                                      :"{+}unknown operator");
        }
        else
        if ((eval_sign == OPER_DIV)
         || (eval_sign == OPER_MOD))
        {
            arg_SetStyle (ARG_STYLE_DIVISION);
        }
    }

    return err;
}



/* read_sign:
 *  Lit un signe.
 */
static int read_sign (void)
{
    int i = 0;
    int err = 0;

    while ((err == 0) && (i >= 0))
    {
        if (*peval == '(')
        {
            arg_ReadChar (&peval, ARG_STYLE_PARENTHESIS_ON);
            err = push_eval (0, PRECEDENCE_PARENTHESIS, 0);
        }
        else
        if (is_eval_end (*peval) != 0)
        {
            err = error_missing_information ();
        }
        else
        {
            i = find_sign (sign_list, ARG_STYLE_SIGN);
            if (i >= 0)
            {
                err = push_eval (eval_sign, eval_precedence, 0);
            }
        }
    }

    return err;
}



/* calculate:
 *  Effectue les calculs.
 */
static int calculate (void)
{
    int err = 0;
    u16 v1;
    u16 v2;

    if (eval_list != NULL)
    {
        v1 = eval_list->value;
        v2 = eval_value;

        switch (eval_list->sign)
        {
            case OPER_MULT:   v1 *= v2; break;
            case OPER_MINUS:  v1 -= v2; break;
            case OPER_PLUS:   v1 += v2; break;
            case OPER_OR:     v1 |= v2; break;
            case OPER_AND:    v1 &= v2; break;
            case OPER_XOR:    v1 ^= v2; break;
            case SIGN_MINUS:  v1 = -v2; break;
            case SIGN_COM:    v1 = ~v2; break;
            case SIGN_PLUS:   v1 += v2; break;
            case SIGN_NOT:    v1 = (u16)(-v2 - 1); break;
            case OPER_LE:     v1 = (u16)((v1 <= v2) ? -1 : 0); break;
            case OPER_EQUAL:  v1 = (u16)((v1 == v2) ? -1 : 0); break;
            case OPER_GT:     v1 = (u16)((v1 > v2) ? -1 : 0); break;
            case OPER_LESS:   v1 = (u16)((v1 < v2) ? -1 : 0); break;
            case OPER_OR2:    v1 = (u16)(((v1 | v2) != 0) ? -1 : 0); break;
            case OPER_GE:     v1 = (u16)((v1 >= v2) ? -1 : 0); break;
            case OPER_AND2:   v1 = (u16)(((v1 & v2) != 0) ? -1 : 0); break;
            case OPER_NE:     v1 = (u16)(((v1 != v2) != 0) ? -1 : 0); break;
            case OPER_RSHIFT: v1 = (u16)(((uint)v1&0xffff)>>((uint)v2&0xffff));
                                    break;
            case OPER_DIV:    v1 /= (v2 == 0) ? 1 : v2; break;
            case OPER_MOD:    v1 %= (v2 == 0) ? 1 : v2; break;
            case OPER_LSHIFT:
                v1 = ((eval_soft==0)&&(assemble.soft<ASSEMBLER_C6809)&&(v2>0x7fff))
                    ? (u16)(v1 >> (u16)(-v2))
                    : (u16)(v1 << v2);
                break;
        }

        err = ((v2 == 0)
            && ((eval_list->sign == OPER_DIV)
             || (eval_list->sign == OPER_MOD)))
                ? error_Error ((is_fr)?"{/}division par 0"
                                      :"{/}division by 0") : 0;
        eval_value = v1;
    }
    else
    {
        err = error_Internal (__FILE__, __LINE__, __func__);
    }

    return err;
}



/* pull_bracket:
 *  Effectue les opérations entre parenthèses.
 */
static int pull_bracket (void)
{
    int err = 0;

    while ((err == 0) && (*peval == ')'))
    {
        arg_ReadChar (&peval, ARG_STYLE_PARENTHESIS_OFF);

        if (eval_list != NULL)
        {
            /* Effectue les opérations entre parenthèses */
            while (eval_list->precedence < PRECEDENCE_PARENTHESIS)
            {
                err |= calculate ();
                pull_eval ();
            }

            if (eval_list->precedence == PRECEDENCE_PARENTHESIS)
            {
                pull_eval ();
            }
            else
            {
                err |= error_Error ((is_fr)?"{-}parenthèse gauche manquante"
                                           :"{-}missing left parenthesis");
            }
        }
        else
        {
            err = error_Internal (__FILE__, __LINE__, __func__);
        }
    }

    return err;
}



/* priority_calculation:
 *  Effectue les opérations prioritaires.
 */
static int priority_calculation (void)
{
    int err = 0;

    if (eval_list != NULL)
    {
        if (eval_list->precedence <= eval_precedence)
        {
            err = calculate ();
            pull_eval ();
        }
    }
    else
    {
        err = error_Internal (__FILE__, __LINE__, __func__);
    }

    return err;
}



/* flush_calculation:
 *  Purge l'évaluation.
 */
static int flush_calculation (void)
{
    int err = 0;

    while (eval_list != NULL)
    {
        err |= (eval_list->precedence == PRECEDENCE_PARENTHESIS)
               ? error_Error ((is_fr)
                   ? "{b}parenthèse droite manquante"
                   : "{b}missing right bracket")
               : (eval_list->precedence != PRECEDENCE_END) 
                   ? calculate () : 0;
        pull_eval ();
    }

    return err;
}
    


/* operand_evaluation:
 *  Evaluation de l'opérande
 */
static int operand_evaluation (void)
{
    int err = 0;

    (void)push_eval (0, PRECEDENCE_END, 0);

    if (is_eval_end (*peval) == 0)
    {
        while (is_eval_end (*peval) == 0)
        {
            err |= read_sign ();
            err |= read_value ();
            err |= pull_bracket ();

            if (is_eval_end (*peval) == 0)
            {
                err |= read_operator ();
                err |= priority_calculation ();
                err |= push_eval (eval_operator, eval_precedence, eval_value);
            }
        }

        err |= flush_calculation ();
    }
    else
    {
        err |= error_missing_information ();
    }

    return err;
}


/* ------------------------------------------------------------------------- */


/* eval_Do:
 *  Evaluation de l'opérande.
 */
int eval_Do (char **p, u16 *value)
{
    int err;

    peval = *p;
    err = operand_evaluation ();
    *p = peval;
    *value = (err == 0) ? eval_value : 0;

    return err;
}



/* eval_SetSoftOperator:
 *  Demande les opérateurs/signes relatifs au soft.
 */
void eval_SetSoftOperator (void)
{
    eval_soft = 0;
}



/* eval_SetC6809Operator:
 *  Demande les opérateurs/signes relatifs à C6809.
 */
void eval_SetC6809Operator (void)
{
    eval_soft = 1;
}

