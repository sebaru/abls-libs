/******************************************************************************************************************************/
/* src/run.c            Helpers d'execution fork/exec partages — abls-libs                                                  */
/* Projet Abls-Habitat version 1.0       Gestion d'habitat                                                22.07.2026          */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * run.c
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

 #include <errno.h>
 #include <stdarg.h>
 #include <sys/wait.h>
 #include <unistd.h>

 #include "abls-libs.h"

 #define ABLS_EXEC_FAILED 127

/******************************************************************************************************************************/
/* Run_shell_real: Lance une commande via fork puis execvp                                                                    */
/* Entree: commande_full       - commande complete a executer (chaine null-terminee)                                           */
/*         wait_for_completion - TRUE pour attendre la fin du processus fils, FALSE pour lancer en arriere-plan               */
/* Sortie: code de retour du processus fils, -1 si erreur interne, 128+signal si tue par signal                               */
/******************************************************************************************************************************/
 static gint Run_shell_real ( const gchar *commande_full, gboolean wait_for_completion )
  { if (!commande_full)
     { Info( __func__, FACILITY_RUN, NULL, LOG_WARNING, "Empty command refused" );
       return(-1);
     }

    GError *error = NULL;
    gchar **argv = NULL;
    if ( g_shell_parse_argv( commande_full, NULL, &argv, &error ) == FALSE )
     { Info( __func__, FACILITY_RUN, NULL, LOG_ALERT, "Memory error building argv for '%s': %s",
             commande_full, error->message );
       g_error_free(error);
       return(-1);
     }

    Info( __func__, FACILITY_RUN, NULL, LOG_NOTICE, "Running command: %s", commande_full );

    pid_t pid = fork();
    if (pid < 0)                                                                                         /* Si erreur de fork */
     { Info( __func__, FACILITY_RUN, NULL, LOG_ERR, "fork failed for '%s': %s", commande_full, g_strerror(errno) );
       goto end;
     }

    if (pid == 0)                                                                                 /* Lancement de la commande */
     { execvp( (gchar *)argv[0], argv );
       Info( __func__, FACILITY_RUN, NULL, LOG_ERR, "execvp failed for '%s': %s", commande_full, g_strerror(errno) );
       _exit(ABLS_EXEC_FAILED);
     }

    Info( __func__, FACILITY_RUN, NULL, LOG_INFO, "Forked pid %d for '%s'", pid, commande_full );             /* Dans le pere */

    if (!wait_for_completion) goto end;                                /* Si on ne veut pas attendre la fin du processus fils */

    gint status = -1;
    while (waitpid(pid, &status, 0) < 0)
     { if (errno == EINTR) continue;
       Info( __func__, FACILITY_RUN, NULL, LOG_ERR, "waitpid failed for '%s': %s", commande_full, g_strerror(errno) );
       goto end;
     }

    if (WIFEXITED(status))
     { status = WEXITSTATUS(status);
       Info( __func__, FACILITY_RUN, NULL, LOG_INFO, "Command '%s' exited with code %d", commande_full, status );
     }
    else if (WIFSIGNALED(status))
     { status = 128 + WTERMSIG(status);
       Info( __func__, FACILITY_RUN, NULL, LOG_ERR, "Command '%s' killed by signal %d", commande_full, WTERMSIG(status) );
       goto end;
     }

end:
    g_strfreev(argv);
    return( status );
  }
/******************************************************************************************************************************/
/* Run_shell: Lance la commande demandee apres formatage printf-like de la chaine                                             */
/* Entree: command       - format de la commande (style printf)                                                               */
/*         ...           - arguments variables pour le formatage                                                              */
/* Sortie: code de retour du processus fils, -1 si erreur interne, 128+signal si tue par signal                               */
/******************************************************************************************************************************/
 gint Run_shell ( const gchar *command, ... )
  { gchar commande_full[256];
    va_list ap;
    va_start(ap, command);
    g_vsnprintf(commande_full, sizeof(commande_full), command, ap);
    va_end(ap);
    return ( Run_shell_real(commande_full, TRUE) );
  }
/******************************************************************************************************************************/
/* Run_shell_detached: Lance la commande en arriere-plan sans attendre sa terminaison                                         */
/* Entree: command       - format de la commande (style printf)                                                               */
/*         ...           - arguments variables pour le formatage                                                              */
/* Sortie: neant                                                                                                              */
/******************************************************************************************************************************/
 void Run_shell_detached ( const gchar *command, ... )
  { gchar commande_full[256];
    va_list ap;
    va_start(ap, command);
    g_vsnprintf(commande_full, sizeof(commande_full), command, ap);
    va_end(ap);
    Run_shell_real(commande_full, FALSE);
  }
/******************************************************************************************************************************/
/* Run_thread_with_join: Lance un thread GLib et attend sa terminaison                                                        */
/* Entree: name          - nom du thread (peut etre NULL)                                                                     */
/*         func          - fonction du thread (GThreadFunc)                                                                   */
/*         data          - donnee passee a la fonction du thread                                                              */
/* Sortie: valeur de retour du thread, NULL si erreur de creation                                                             */
/******************************************************************************************************************************/
 gpointer Run_thread_with_join ( const gchar *name, GThreadFunc func, gpointer data )
  { if (!name) name = "New_joined_thread";
    GError *error = NULL;
    GThread *thread = g_thread_try_new ( name, func, data, &error );
    if (!thread)
     { Info( __func__, FACILITY_RUN, name, LOG_ERR, "g_thread_try_new failed for '%s': %s", name, error ? error->message : "unknown error" );
       if (error) g_error_free(error);
       return(NULL);
     }
    Info( __func__, FACILITY_RUN, name, LOG_INFO, "Thread '%s' started, waiting for join", name );
    gpointer retval = g_thread_join ( thread );
    Info( __func__, FACILITY_RUN, name, LOG_INFO, "Thread '%s' joined", name );
    return(retval);
  }
/******************************************************************************************************************************/
/* Run_thread_detached: Lance un thread GLib en mode detache                                                                  */
/* Entree: name          - nom du thread (peut etre NULL)                                                                     */
/*         func          - fonction du thread (GThreadFunc)                                                                   */
/*         data          - donnee passee a la fonction du thread                                                              */
/* Sortie: TRUE si OK                                                                                                         */
/******************************************************************************************************************************/
 gboolean Run_thread_detached ( const gchar *name, GThreadFunc func, gpointer data )
  { if (!name) name = "New_detached_thread";
    GError *error = NULL;
    GThread *thread = g_thread_try_new ( name, func, data, &error );
    if (!thread)
     { Info( __func__, FACILITY_RUN, name, LOG_ERR, "g_thread_try_new failed for '%s': %s", name, error ? error->message : "unknown error" );
       if (error) g_error_free(error);
       return(FALSE);
     }
    Info( __func__, FACILITY_RUN, name, LOG_INFO, "Thread '%s' started (detached)", name );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Run_thread_pool_init: Initialise un pool de threads GLib                                                                   */
/* Entree: name          - nom du pool (NULL pour "default_pool")                                                             */
/*         pool_fonction - fonction a executer pour chaque tache (GFunc)                                                      */
/*         pool_data     - donnees utilisateur transmises a chaque appel de pool_fonction                                     */
/*         max_threads   - nombre maximum de threads dans le pool                                                             */
/* Sortie: pointeur sur ABLS_RUN_POOL si succes, NULL si erreur                                                               */
/******************************************************************************************************************************/
 struct ABLS_RUN_POOL *Run_thread_pool_init ( gchar *name, GFunc pool_fonction, gpointer pool_data, guint max_threads )
  { if (!name) name = "New_pool";
    if (!pool_fonction) { Info ( __func__, FACILITY_RUN, name, LOG_ERR, "Function is NULL" ); return(NULL); }

    struct ABLS_RUN_POOL *abls_pool = g_try_malloc0 ( sizeof (struct ABLS_RUN_POOL) );
    if (!abls_pool) { Info ( __func__, FACILITY_RUN, name, LOG_ERR, "Unable to allocate memory for abls_pool" ); return(NULL); }

    abls_pool->g_pool = g_thread_pool_new ( pool_fonction, pool_data, max_threads, TRUE, NULL );
    if (!abls_pool->g_pool)
     { Info ( __func__, FACILITY_RUN, name, LOG_ERR, "Unable to allocate memory for abls_pool->g_pool" );
       g_free(abls_pool);
       return(NULL);
     }
    g_snprintf ( abls_pool->name, sizeof(abls_pool->name), "%s", name );
    abls_pool->max_threads    = max_threads;
    abls_pool->pool_fonction  = pool_fonction;
    abls_pool->pool_data      = pool_data;
    Info ( __func__, FACILITY_RUN, abls_pool->name, LOG_INFO, "Starting pool with '%d' threads", abls_pool->max_threads );
    return(abls_pool);
  }
/******************************************************************************************************************************/
/* Run_thread_pool_push: Ajoute une tache au pool de threads pour execution                                                   */
/* Entree: abls_pool     - pool initialise par Run_thread_pool_init                                                           */
/*         fonction_data - donnees a traiter par la fonction du pool                                                          */
/* Sortie: neant                                                                                                              */
/******************************************************************************************************************************/
 void Run_thread_pool_push ( struct ABLS_RUN_POOL *abls_pool, gpointer fonction_data )
  { if (!abls_pool) { Info ( __func__, FACILITY_RUN, NULL, LOG_ERR, "Pool is NULL" ); return; }
    if (!g_thread_pool_push ( abls_pool->g_pool, fonction_data, NULL ))
     { Info ( __func__, FACILITY_RUN, abls_pool->name, LOG_ERR, "Unable to push data to pool" ); }
  }
/******************************************************************************************************************************/
/* Run_thread_pool_end: Termine et libere les ressources du pool de threads                                                   */
/* Entree: abls_pool           - pool a terminer                                                                              */
/*         wait_for_completion - TRUE pour attendre les threads en cours, FALSE sinon                                         */
/* Sortie: neant                                                                                                              */
/******************************************************************************************************************************/
 void Run_thread_pool_end ( struct ABLS_RUN_POOL *abls_pool, gboolean wait_for_completion )
  { if (!abls_pool) { Info ( __func__, FACILITY_RUN, NULL, LOG_ERR, "Pool is NULL" ); return; }
    g_thread_pool_free ( abls_pool->g_pool, FALSE, wait_for_completion );
    Info ( __func__, FACILITY_RUN, abls_pool->name, LOG_INFO, "Pool ended" );
    g_free ( abls_pool );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/