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

#ifndef C6809_OPT_H
#define C6809_OPT_H 1

#define OPT_NO  (1 << 0)  /* Pas d'objet */
#define OPT_OP  (1 << 1)  /* Demande d'optimisation */
#define OPT_SS  (1 << 2)  /* Ligne séparées (pas d'effet) */
#define OPT_WE  (1 << 3)  /* Attente à l'erreur (pas d'effet) */
#define OPT_WL  (1 << 4)  /* Affichage des lignes */
#define OPT_WS  (1 << 5)  /* Affichage de la liste des symboles */

extern void  opt_Init (void);
extern void  opt_Reset (void);
extern void  opt_AssembleOPT (void);
extern void  opt_SetOptimize (void);
extern int   opt_GetUsr (int flag);
extern void  opt_SetIni (int flag, int status);

#endif

