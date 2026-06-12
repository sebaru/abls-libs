/******************************************************************************************************************************/
/* include/Lifo.h      Declarations du LIFO circulaire thread-safe — abls-libs                                               */
/* Projet Abls-Habitat version 1.0       Gestion d'habitat                                                12.06.2026          */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Lifo.h
 * This file is part of Abls-Libs
 *
 * Copyright (C) 1988-2026 - Sebastien LEFEVRE
 *
 * Abls-Libs is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Abls-Libs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Abls-Libs; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef _LIFO_H_
 #define _LIFO_H_

 #include <glib.h>

 typedef struct _ABLS_LIFO_NODE ABLS_LIFO_NODE;

 struct _ABLS_LIFO_NODE
  { ABLS_LIFO_NODE *next;
    gpointer data;
  };

 typedef struct
  { ABLS_LIFO_NODE *newer;                                                 /* Pointeur vers le noeud le plus récemment ajouté */
    ABLS_LIFO_NODE *older;                                                           /* Pointeur vers le noeud le plus ancien */
    GRWLock lock;
    guint size;
  } ABLS_LIFO;

 extern ABLS_LIFO *abls_lifo_push ( ABLS_LIFO *lifo, gpointer data );
 extern gpointer abls_lifo_pop ( ABLS_LIFO *lifo );
 extern guint abls_lifo_size ( ABLS_LIFO *lifo );
 extern void abls_lifo_destroy ( ABLS_LIFO *lifo );
 extern void abls_lifo_destroy_with_free ( ABLS_LIFO *lifo );

#endif /* _LIFO_H_ */
/*----------------------------------------------------------------------------------------------------------------------------*/
