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
    #include <string.h>
#endif

#include "defs.h"
#include "error.h"
#include "arg.h"
#include "assemble.h"
#include "output.h"
#include "directiv/opt.h"

struct OPTION_NAME_LIST {
    char name[3];  /* Nom de l'option */
    int  flag;     /* Flag de l'option */
};

static struct OPTION_NAME_LIST option_list[] = {
   { "NO", OPT_NO }, /* Pas d'objet sauvegardé dans le BIN */
   { "OP", OPT_OP }, /* Conseil d'optimisation */
   { "SS", OPT_SS }, /* Ligne séparées (non actif) */
   { "WE", OPT_WE }, /* Attente à l'erreur (non actif) */
   { "WL", OPT_WL }, /* Affichage des lignes (non actif) */
   { "WS", OPT_WS }, /* Affichage des symboles dans le fichier lst */
   { ""  , 0      }  /* - Fin de liste - */
};

static int opt_ini = 0;
static int opt_usr = 0;



/* get_option_index:
 *  Renvoie l'index de l'option.
 */
static int get_option_index (void)
{
    int i;
    int index = -1;
    char *upper_name = arg_Upper (arg.str);

    for (i=0; (option_list[i].name[0] != '\0') && (index == -1); i++)
    {
        index = (strcmp (upper_name, option_list[i].name) == 0) ? i : -1;
    }

    return index;
}



/* read_options:
 *  Lit les options.
 */
static void read_options (void)
{
    int i;
    int err = 0;
    int status;

    while (err == 0)
    {
        /* Positionne le status */
        status = 1;
        if (*assemble.ptr == '.')
        {
            (void)arg_Read (&assemble.ptr, ARG_STYLE_SIGN);
            status = 0;
        }

        /* Lit l'option */
        if (arg_Read (&assemble.ptr, ARG_STYLE_NONE) == ARG_END)
        {
            err = error_Error ((is_fr)?"{-}option manquante"
                                      :"{-}missing option");
        }
        else
        {
            i = get_option_index ();
            if (i >= 0)
            {
                opt_usr &= ~option_list[i].flag;
                opt_usr |= (status == 1) ? option_list[i].flag : 0;
                if (*assemble.ptr == '/')
                {
                    (void)arg_Read (&assemble.ptr, ARG_STYLE_SIGN);
                }
                else
                err = (arg_Read (&assemble.ptr, ARG_STYLE_SIGN) != ARG_END)
                      ? error_BadSeparator () : 1;
            }
            else
            {
                err = error_Error ((is_fr)?"{+}option inconnue"
                                          :"{+}unknown option");
            }
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


/* opt_AssembleOPT:
 *  Assemble la directive OPT.
 */
void opt_AssembleOPT (void)
{
    allow ();
    read_options ();
}



/* opt_SetOptimize:
 *  Demande les conseils d'optimisation.
 */
void opt_SetOptimize (void)
{
    opt_ini |= OPT_OP;
}



/* opt_GetUsr:
 *  Retourne l'état du flag définit par l'utilisateur.
 */
int opt_GetUsr (int flag)
{
    return ((opt_usr & flag) == 0) ? 0 : 1;
}



/* opt_SetIni:
 *  Active/Désactive l'état du flag déini pour l'initialisation.
 */
void opt_SetIni (int flag, int status)
{
    opt_ini &= ~flag;
    opt_ini |= (status == 0) ? 0 : flag;
}



/* opt_Init:
 *  Initialise le module OPT.
 */
void opt_Init (void)
{
    opt_ini = 0;
    opt_usr = 0;
}



/* opt_Reset:
 *  Initialise le module OPT utilisateur.
 */
void opt_Reset (void)
{
    opt_usr = opt_ini;
}

