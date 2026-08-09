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
/* Entree: commande_full - commande complete a executer (chaine null-terminee)                                                */
/* Sortie: code de retour du processus fils, -1 si erreur interne, 128+signal si tue par signal                               */
/******************************************************************************************************************************/
 static gint Run_shell_real ( const gchar *commande_full )
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

    gint status;
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
    return ( Run_shell_real(commande_full) );
  }
/******************************************************************************************************************************/
/* Run_thread_join: Lance un thread GLib et attend sa terminaison                                                             */
/* Entree: name          - nom du thread (peut etre NULL)                                                                     */
/*         func          - fonction du thread (GThreadFunc)                                                                   */
/*         data          - donnee passee a la fonction du thread                                                              */
/* Sortie: valeur de retour du thread, NULL si erreur de creation                                                             */
/******************************************************************************************************************************/
 gpointer Run_thread_join ( const gchar *name, GThreadFunc func, gpointer data )
  { if (!name) name = "New_thread";
    GThread *thread = g_thread_new ( name, func, data );
    if (!thread)
     { Info( __func__, FACILITY_RUN, name, LOG_ERR, "g_thread_new failed for '%s'", name );
       return(NULL);
     }
    Info( __func__, FACILITY_RUN, name, LOG_INFO, "Thread '%s' started, waiting for join", name );
    gpointer retval = g_thread_join ( thread );
    Info( __func__, FACILITY_RUN, name, LOG_INFO, "Thread '%s' joined", name );
    return(retval);
  }

/*----------------------------------------------------------------------------------------------------------------------------*/