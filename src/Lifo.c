/******************************************************************************************************************************/
/* src/Lifo.c      LIFO circulaire thread-safe pour Abls-Libs                                                                 */
/* Projet Abls-Libs version 1.0       Gestion d'habitat                                                            12.06.2026 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Lifo.c
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

 #include "Lifo.h"

/******************************************************************************************************************************/
/* abls_lifo_push: Ajoute un element apres tail puis devient le nouveau tail                                                  */
/* Entree: lifo et data utilisateur                                                                                           */
/* Sortie: pointeur vers la structure ABLS_LIFO ou NULL en cas d'erreur memoire                                               */
/******************************************************************************************************************************/
 ABLS_LIFO *abls_lifo_push ( ABLS_LIFO *lifo, gpointer data )
  { if (!lifo)                                                             /* Si pointeur NULL, alloue une nouvelle structure */
     { ABLS_LIFO *lifo = g_try_malloc0 ( sizeof(ABLS_LIFO) );
       if (!lifo) return NULL;
       g_rw_lock_init ( &lifo->lock );                                            /* Initialise le verrou en lecture/écriture */
     }

    ABLS_LIFO_NODE *node = g_try_malloc0 ( sizeof(ABLS_LIFO_NODE) );                                      /* Création du node */
    if (!node) return(lifo);
    node->data = data;

    g_rw_lock_writer_lock ( &lifo->lock );
    if (!lifo->older)
     { lifo->older = node; }                   /* Premier element de la liste donc le plus vieux et le plus jeune a la fois ? */
    else
     { node->next = lifo->newer; }
    lifo->newer = node;                                                         /* Le nouveau noeud ajouté est le plus récent */
    lifo->size++;
    g_rw_lock_writer_unlock ( &lifo->lock );
    return( lifo );
  }
/******************************************************************************************************************************/
/* abls_lifo_pop: Supprime le dernier élément a être entré dans la liste lifo                                                 */
/* Entree: lifo                                                                                                               */
/* Sortie: data retiree ou NULL si liste vide/erreur                                                                          */
/******************************************************************************************************************************/
 gpointer abls_lifo_pop ( ABLS_LIFO *lifo )
  { if (!lifo) return NULL;
    if (!lifo->older) return NULL;

    g_rw_lock_writer_lock ( &lifo->lock );
    ABLS_LIFO_NODE *older = lifo->older;
    gpointer result = older->data;
    lifo->older = lifo->older->next;
    if (lifo->size) lifo->size--;
    g_rw_lock_writer_unlock ( &lifo->lock );

    g_free ( older );
    return result;
  }
/******************************************************************************************************************************/
/* abls_lifo_size: Retourne le nombre d'éléments dans la liste lifo                                                           */
/* Entree: lifo                                                                                                               */
/* Sortie: nombre d'éléments dans la liste                                                                                    */
/******************************************************************************************************************************/
 guint abls_lifo_size ( ABLS_LIFO *lifo )
  { if (!lifo) return 0;
    return lifo->size;
  }
/******************************************************************************************************************************/
/* abls_lifo_destroy: Supprime tous les noeuds de la liste                                                                    */
/* Entree: lifo                                                                                                               */
/* Sortie: neant                                                                                                              */
/******************************************************************************************************************************/
 void abls_lifo_destroy ( ABLS_LIFO *lifo )
  { if (!lifo) return;

    g_rw_lock_writer_lock ( &lifo->lock );
    while(lifo->older)
     { ABLS_LIFO_NODE *older = lifo->older;
       lifo->older = lifo->older->next;
       g_free ( older );
     }
    g_rw_lock_writer_unlock ( &lifo->lock );
    g_rw_lock_clear ( &lifo->lock );
    g_free ( lifo );
  }
/******************************************************************************************************************************/
/* abls_lifo_destroy_with_free: Supprime les noeuds et applique g_free sur chaque data                                        */
/* Entree: lifo                                                                                                               */
/* Sortie: neant                                                                                                              */
/******************************************************************************************************************************/
 void abls_lifo_destroy_with_free ( ABLS_LIFO *lifo )
  { if (!lifo) return;

    g_rw_lock_writer_lock ( &lifo->lock );
    while(lifo->older)
     { ABLS_LIFO_NODE *older = lifo->older;
       lifo->older = lifo->older->next;
       if (older->data) g_free ( older->data );
       g_free ( older );
     }
    g_rw_lock_writer_unlock ( &lifo->lock );
    g_rw_lock_clear ( &lifo->lock );
    g_free ( lifo );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
