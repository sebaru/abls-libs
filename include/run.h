/******************************************************************************************************************************/
/* include/run.h       Declaration des helpers d'execution fork/exec — abls-libs                                            */
/* Projet Abls-Habitat version 1.0       Gestion d'habitat                                                22.07.2026          */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * run.h
 * This file is part of Abls-Libs
 *
 * Copyright (C) 1988-2026 - Sébastien LEFÈVRE
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

#ifndef _ABLS_RUN_H_
 #define _ABLS_RUN_H_

 #include <glib.h>

 #define FACILITY_RUN  "run"

 struct ABLS_RUN_POOL
  { GThreadPool *g_pool;
    gchar       name[128];
    guint       max_threads;
    GFunc       pool_fonction;
    gpointer    pool_data;
    guint       chrono;
  };

/*-- Execution de processus --------------------------------------------------------------------------------------------------*/
/* Les arguments doivent etre termines par NULL. Retourne le code de sortie du programme lance, -1 si erreur interne.       */
 extern gint     Run_shell       ( const gchar *command, ... );

/*-- Execution de threads ----------------------------------------------------------------------------------------------------*/
 extern gpointer             Run_thread_with_join  ( const gchar *name, GThreadFunc func, gpointer data );
 extern gboolean             Run_thread_detached ( const gchar *name, GThreadFunc func, gpointer data );
 extern struct ABLS_RUN_POOL *Run_thread_pool_init ( gchar *name, GFunc pool_fonction, gpointer pool_data, guint max_threads );
 extern void                 Run_thread_pool_push ( struct ABLS_RUN_POOL *abls_pool, gpointer fonction_data );
 extern void                 Run_thread_pool_end  ( struct ABLS_RUN_POOL *abls_pool, gboolean wait_for_completion );

#endif /* _ABLS_RUN_H_ */
/*----------------------------------------------------------------------------------------------------------------------------*/