/******************************************************************************************************************************/
/* src/Erreur.c      Librairie partagée entre l'API et les Agents — Abls-Habitat                                              */
/* Projet Abls-Habitat-Libs version 1.0       Gestion d'habitat                                                    10.06.2026 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Erreur.c
 * This file is part of Abls-Habitat-Libs
 *
 * Copyright (C) 1988-2026 - Sébastien LEFÈVRE
 *
 * Watchdog is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Watchdog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Watchdog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

 #include "Erreur.h"

/*-- Variables internes (thread-safe) ----------------------------------------*/
 static gint    Nbr_log_sent   = 0;
 static GSList *Debug_contexts = NULL;
 static GRWLock Debug_contexts_lock;
 static guint   Log_level      = LOG_INFO;

/******************************************************************************************************************************/
/* Info_debug_context: Active le forçage de debug pour un contexte donné                                                      */
/* Entrée: le nom du contexte (ex: "smsg", "bus", "json")                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Info_debug_context ( const gchar *context )
  { if (!context) return;
    g_rw_lock_writer_lock ( &Debug_contexts_lock );
    if (! g_slist_find_custom ( Debug_contexts, context, (GCompareFunc)g_strcmp0 ) )
     { Debug_contexts = g_slist_prepend ( Debug_contexts, g_strdup(context) );
       Info_new ( __func__, "log", LOG_NOTICE, "master", "Debug Context '%s' is forced", context );
     }
    g_rw_lock_writer_unlock ( &Debug_contexts_lock );
  }
/******************************************************************************************************************************/
/* Info_undebug_context: Désactive le forçage de debug pour un contexte donné                                                 */
/* Entrée: le nom du contexte                                                                                                 */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Info_undebug_context ( const gchar *context )
  { if (!context) return;
    g_rw_lock_writer_lock ( &Debug_contexts_lock );
    GSList *found = g_slist_find_custom ( Debug_contexts, context, (GCompareFunc)g_strcmp0 );
    if (found)
     { g_free ( found->data );
       Debug_contexts = g_slist_delete_link ( Debug_contexts, found );
       Info_new ( __func__, "log", LOG_NOTICE, "master", "Debug Context '%s' is no longer forced", context );
     }
    g_rw_lock_writer_unlock ( &Debug_contexts_lock );
  }
/******************************************************************************************************************************/
/* Info_clear_debug_contexts: Vide la liste de tous les contextes de debug forcés                                             */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Info_clear_debug_contexts ( void )
  { g_rw_lock_writer_lock ( &Debug_contexts_lock );
    if (Debug_contexts)
     { g_slist_free_full ( Debug_contexts, g_free );
       Debug_contexts = NULL;
     }
    g_rw_lock_writer_unlock ( &Debug_contexts_lock );
  }
/******************************************************************************************************************************/
/* Info_reset_nbr_log: Retourne et remet à zéro le compteur de messages de log                                                */
/* Entrée: néant                                                                                                              */
/* Sortie: le nombre de messages envoyés depuis le dernier appel                                                              */
/******************************************************************************************************************************/
 gint Info_reset_nbr_log ( void )
  { return ( g_atomic_int_and ( &Nbr_log_sent, 0 ) ); }
/******************************************************************************************************************************/
/* Info_new: Envoie un message structuré JSON vers syslog                                                                     */
/* Entrée: function  — nom de la fonction appelante (__func__)                                                                */
/*         context   — sous-système / module (ex: "smsg", "json") ; NULL → ""                                                 */
/*         priority  — niveau syslog (LOG_ERR, LOG_WARNING, LOG_NOTICE, LOG_INFO, LOG_DEBUG)                                  */
/*         domain_uuid — UUID du domaine applicatif ; NULL → "master"                                                         */
/*         format    — format printf suivi des arguments variadiques                                                          */
/* Sortie: néant                                                                                                              */
/*                                                                                                                            */
/* Comportement de filtrage :                                                                                                 */
/*   - Si le contexte figure dans Debug_contexts → le message est toujours émis                                               */
/*   - Sinon → syslog filtre normalement selon setlogmask                                                                     */
/******************************************************************************************************************************/
 void Info_new ( const gchar *function, const gchar *context, guint priority,
                 const gchar *domain_uuid, const gchar *format, ... )
  { gchar chaine[512], nom_thread[32];
    gboolean forced;
    va_list ap;

    if (context)
     { g_rw_lock_reader_lock ( &Debug_contexts_lock );
       if (g_slist_find_custom ( Debug_contexts, context, (GCompareFunc)g_strcmp0 ) ) forced = TRUE;
       g_rw_lock_reader_unlock ( &Debug_contexts_lock );
     } else forced = FALSE;

    if (!forced && priority > Log_level) return;

    prctl ( PR_GET_NAME, &nom_thread, 0, 0, 0 );
    g_snprintf ( chaine, sizeof(chaine),
                 "{ \"domain_uuid\": \"%s\", \"context\":\"%s\", \"thread\":\"%s\", \"function\":\"%s\", \"message\":\"%s\" }",
                 (domain_uuid ? domain_uuid : "master"),
                 (context     ? context     : "none"),
                 nom_thread,
                 (function ? function : "none"),
                 (format ? format : "none") );

    va_start ( ap, format );
    vsyslog ( priority, chaine, ap );
    va_end ( ap );

    g_atomic_int_inc ( &Nbr_log_sent );
  }
/******************************************************************************************************************************/
/* Info_change_log_level: Change le niveau de log global                                                                      */
/* Entrée: Le nouveau niveau (LOG_DEBUG, LOG_INFO, etc.)                                                                      */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Info_change_log_level ( guint new_log_level )
  { Log_level = new_log_level;
    Info_new ( __func__, "log", LOG_NOTICE, "master", "Log level set to %d", new_log_level );
  }
/******************************************************************************************************************************/
/* Info_stop: Ferme la connexion syslog                                                                                       */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Info_stop ( int code_retour, void *data )
  { Info_new ( __func__, "log", LOG_NOTICE, "master", "End of logs" );
    Info_clear_debug_contexts ();
    g_rw_lock_clear ( &Debug_contexts_lock );
    closelog();
  }
/******************************************************************************************************************************/
/* Info_init: Initialise le sous-système de log                                                                               */
/* Entrée: entete    — identifiant du processus dans syslog (NULL → nom du processus)                                         */
/*         log_level — niveau initial (LOG_DEBUG = 7, LOG_INFO = 6, …)                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Info_init ( const gchar *entete, guint log_level )
  { g_rw_lock_init ( &Debug_contexts_lock );
    Debug_contexts = NULL;
    on_exit( Info_stop, NULL );
    openlog ( entete, LOG_CONS | LOG_PID, LOG_USER );
    Info_new ( __func__, "log", LOG_INFO, "master", "Start of logs" );
    Info_change_log_level ( log_level );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
