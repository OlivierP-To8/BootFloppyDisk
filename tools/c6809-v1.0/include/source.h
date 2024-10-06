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

#ifndef C6809_SOURCE_H
#define C6809_SOURCE_H 1

struct SOURCE_TEXT {
    char  /*@null@*/*str;  /* Texte de la ligne */
    struct SOURCE_TEXT /*@only@*//*@null@*/*next;
};

struct SOURCE_LIST {
    int   type;       /* Type de fichier */
    char  /*@only@*//*@null@*/*pc;  /* Chemin PC du source */
    char  /*@only@*//*@null@*/*thomson;  /* Chemin Thomson du source */
    int   encoding;   /* Encodage du texte */
    int   line;       /* Numéro de première ligne */
    int   used;       /* Flag d'utilisation */
    struct SOURCE_TEXT /*@owned@*//*@null@*/*text;  /* Liste des lignes */
    struct SOURCE_LIST /*@only@*//*@null@*/*next;
};

extern struct SOURCE_LIST /*@only@*//*@null@*/*source_list;

extern struct SOURCE_LIST /*@dependent@*//*@null@*/*source_Includ (void);
extern struct SOURCE_LIST /*@dependent@*//*@null@*/*source_Load (char *name);
extern int   source_GlobalEncoding (void);
extern void  source_Close (void);
extern int   source_OpenBin (char *extension);

#endif

