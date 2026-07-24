/******************************************************************************************************************************/
/* src/fork.c           Helpers d'execution fork/exec partages — abls-libs                                                  */
/* Projet Abls-Habitat version 1.0       Gestion d'habitat                                                22.07.2026          */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * fork.c
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
/* Exec_build_argv: Construit un argv NULL-termine a partir des arguments variadiques                                        */
/******************************************************************************************************************************/
 static GPtrArray *Exec_build_argv ( gboolean use_sudo, const gchar *command, va_list ap )
  { GPtrArray *argv;
    const gchar *arg;

    if (!command || !command[0]) return(NULL);

    argv = g_ptr_array_new();
    if (!argv) return(NULL);

    if (use_sudo)
     { g_ptr_array_add(argv, (gpointer)"sudo");
       g_ptr_array_add(argv, (gpointer)"-n");
     }

    g_ptr_array_add(argv, (gpointer)command);
    while ( (arg = va_arg(ap, const gchar *)) != NULL )
     { g_ptr_array_add(argv, (gpointer)arg); }
    g_ptr_array_add(argv, NULL);
    return(argv);
  }
/******************************************************************************************************************************/
/* Exec_wait_child: Attend le fils et traduit le statut en code de retour exploitable                                        */
/******************************************************************************************************************************/
 static gint Exec_wait_child ( pid_t pid, const gchar *command )
  { gint status;

    while (waitpid(pid, &status, 0) < 0)
     { if (errno == EINTR) continue;
       Info( __func__, FACILITY_FORK, NULL, LOG_ERR, "waitpid failed for '%s': %s", command, g_strerror(errno) );
       return(-1);
     }

    if (WIFEXITED(status))
     { gint retour = WEXITSTATUS(status);
       Info( __func__, FACILITY_FORK, NULL, LOG_NOTICE, "Command '%s' exited with code %d", command, retour );
       return(retour);
     }

    if (WIFSIGNALED(status))
     { gint retour = 128 + WTERMSIG(status);
       Info( __func__, FACILITY_FORK, NULL, LOG_ERR, "Command '%s' killed by signal %d", command, WTERMSIG(status) );
       return(retour);
     }

    Info( __func__, FACILITY_FORK, NULL, LOG_ERR, "Command '%s' ended with unsupported wait status", command );
    return(-1);
  }
/******************************************************************************************************************************/
/* Exec_run_internal: Lance une commande via fork puis execvp, eventuellement via sudo -n                                   */
/******************************************************************************************************************************/
 static gint Exec_run_internal ( gboolean use_sudo, const gchar *command, va_list ap )
  { if (!command || !command[0])
     { Info( __func__, FACILITY_FORK, NULL, LOG_WARNING, "Empty command refused" );
       return(-1);
     }

    GPtrArray *argv = Exec_build_argv(use_sudo, command, ap);
    if (!argv)
     { Info( __func__, FACILITY_FORK, NULL, LOG_ALERT, "Memory error building argv for '%s'", command );
       return(-1);
     }

    gchar *commande = g_strjoinv(" ", (gchar **)argv->pdata);
    Info( __func__, FACILITY_FORK, NULL, LOG_NOTICE, "Launching command: %s", commande ? commande : command );

    pid_t pid = fork();
    if (pid < 0)
     { Info( __func__, FACILITY_FORK, NULL, LOG_ERR, "fork failed for '%s': %s", command, g_strerror(errno) );
       if (commande) g_free(commande);
       g_ptr_array_free(argv, TRUE);
       return(-1);
     }

    if (pid == 0)
     { execvp( (gchar *)argv->pdata[0], (gchar * const *)argv->pdata );
       Info( __func__, FACILITY_FORK, NULL, LOG_ERR, "execvp failed for '%s': %s", command, g_strerror(errno) );
       _exit(ABLS_EXEC_FAILED);
     }

    Info( __func__, FACILITY_FORK, NULL, LOG_NOTICE, "Forked pid %d for '%s'", pid, command );
    if (commande) g_free(commande);
    g_ptr_array_free(argv, TRUE);
    return( Exec_wait_child(pid, command) );
  }
/******************************************************************************************************************************/
/* Exec: Lance directement la commande demandee                                                                              */
/******************************************************************************************************************************/
 gint Exec ( const gchar *command, ... )
  { va_list ap;
    gint retour;

    va_start(ap, command);
    retour = Exec_run_internal(FALSE, command, ap);
    va_end(ap);
    return(retour);
  }
/******************************************************************************************************************************/
/* Exec_sudo: Lance la commande demandee en la prefixant par sudo -n                                                         */
/******************************************************************************************************************************/
 gint Exec_sudo ( const gchar *command, ... )
  { va_list ap;
    gint retour;

    va_start(ap, command);
    retour = Exec_run_internal(TRUE, command, ap);
    va_end(ap);
    return(retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/