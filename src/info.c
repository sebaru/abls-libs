/******************************************************************************************************************************/
/* src/info.c      Librairie partagee de journalisation abls_info — Abls-Habitat                                            */
/* Projet Abls-Libs version 1.0       Gestion d'habitat                                                            13.06.2026 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * info.c
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

 #include "info.h"

/*-- Variables internes (thread-safe) ----------------------------------------*/
 static gint    Nbr_log_sent    = 0;
 static GSList *Debug_facilities = NULL;
 static GRWLock Debug_facilities_lock;
 static guint   Log_level       = LOG_INFO;
 static const gchar *Prefixe_name = NULL;

/******************************************************************************************************************************/
/* abls_info_debug_facility: Active le forcage de debug pour une facility donnée                                              */
/* Entree: le nom du facility (ex: "smsg", "bus", "json")                                                                     */
/* Sortie: neant                                                                                                              */
/******************************************************************************************************************************/
 void Info_debug_facility ( const gchar *prefixe_valeur, const gchar *facility )
  { if (!facility) return;
    g_rw_lock_writer_lock ( &Debug_facilities_lock );
    if (! g_slist_find_custom ( Debug_facilities, facility, (GCompareFunc)g_strcmp0 ) )
     { Debug_facilities = g_slist_prepend ( Debug_facilities, g_strdup(facility) );
       Info_with_prefix ( __func__, "log", prefixe_valeur, LOG_NOTICE, "Debug facility '%s' is forced", facility );
     }
    g_rw_lock_writer_unlock ( &Debug_facilities_lock );
  }
/******************************************************************************************************************************/
/* Info_undebug_facility: Desactive le forcage de debug pour une facility donnée                                              */
/* Entree: le nom du facility (ex: "smsg", "bus", "json")                                                                     */
/* Sortie: neant                                                                                                              */
/******************************************************************************************************************************/
 void Info_undebug_facility ( const gchar *prefixe_valeur, const gchar *facility )
  { if (!facility) return;
    g_rw_lock_writer_lock ( &Debug_facilities_lock );
    GSList *found = g_slist_find_custom ( Debug_facilities, facility, (GCompareFunc)g_strcmp0 );
    if (found)
     { g_free ( found->data );
       Debug_facilities = g_slist_delete_link ( Debug_facilities, found );
       Info_with_prefix ( __func__, "log", "master", LOG_NOTICE, "Debug facility '%s' is no longer forced", facility );
     }
    g_rw_lock_writer_unlock ( &Debug_facilities_lock );
  }
/******************************************************************************************************************************/
/* Info_clear_debug_facilities: Vide la liste de tous les facilities de debug forces                                          */
/* Entree: neant                                                                                                              */
/* Sortie: neant                                                                                                              */
/******************************************************************************************************************************/
 void Info_clear_debug_facilities ( void )
  { g_rw_lock_writer_lock ( &Debug_facilities_lock );
    if (Debug_facilities)
     { g_slist_free_full ( Debug_facilities, g_free );
       Debug_facilities = NULL;
     }
    g_rw_lock_writer_unlock ( &Debug_facilities_lock );
  }
/******************************************************************************************************************************/
/* Info_reset_nbr_log: Retourne et remet a zero le compteur de messages de log                                                */
/* Entree: neant                                                                                                              */
/* Sortie: le nombre de messages envoyes depuis le dernier appel                                                              */
/******************************************************************************************************************************/
 gint Info_reset_nbr_log ( void )
  { return ( g_atomic_int_and ( &Nbr_log_sent, 0 ) ); }
/******************************************************************************************************************************/
/* Info_full: Envoie un message structure JSON vers syslog                                                                    */
/* Entree: les prefixes, le format, les variables variadiques                                                                 */
/* Sortie: neant                                                                                                              */
/*                                                                                                                            */
/* Comportement de filtrage :                                                                                                 */
/*   - Si le facility figure dans Debug_facilities -> le message est toujours emis                                            */
/*   - Sinon -> la priority est comparee au Log_level                                                                         */
/******************************************************************************************************************************/
 static void Info_full ( const gchar *function, const gchar *prefixe,
                              const gchar *facility, guint priority, const gchar *format, va_list ap )
  { gchar resultat[512], chaine[128], nom_thread[32];
    gboolean forced;

    if (facility)
     { g_rw_lock_reader_lock ( &Debug_facilities_lock );
       if (g_slist_find_custom ( Debug_facilities, facility, (GCompareFunc)g_strcmp0 ) ) forced = TRUE;
       g_rw_lock_reader_unlock ( &Debug_facilities_lock );
     } else forced = FALSE;

    if (!forced && priority > Log_level) return;

    prctl ( PR_GET_NAME, &nom_thread, 0, 0, 0 );
    g_snprintf ( resultat, sizeof(resultat), "{ \"thread\": \"%s\", ", nom_thread );
    if (Prefixe_name && prefixe)
     { g_snprintf ( chaine, sizeof(chaine), "\"%s\": \"%s\", ", Prefixe_name, prefixe );
       g_strlcat ( resultat, chaine, sizeof(resultat) );
     }
    if (facility)
     { g_snprintf ( chaine, sizeof(chaine), "\"facility\": \"%s\", ", facility );
       g_strlcat ( resultat, chaine, sizeof(resultat) );
     }
    if (function)
     { g_snprintf ( chaine, sizeof(chaine), "\"function\": \"%s\", ", function );
       g_strlcat ( resultat, chaine, sizeof(resultat) );
     }
    if (format)
     { g_snprintf ( chaine, sizeof(chaine), "\"message\": \"%s\" ", format );
       g_strlcat ( resultat, chaine, sizeof(resultat) );
     }
    g_strlcat ( resultat, " }", sizeof(resultat) );

    vsyslog ( priority, resultat, ap );

    g_atomic_int_inc ( &Nbr_log_sent );
  }
/******************************************************************************************************************************/
/* Info: Envoie un message structure JSON vers syslog                                                                         */
/* Entree: function  - nom de la fonction appelante (__func__)                                                                */
/*         facility   - sous-systeme / module (ex: "smsg", "json") ; NULL -> ""                                               */
/*         priority  - niveau syslog (LOG_ERR, LOG_WARNING, LOG_NOTICE, LOG_INFO, LOG_DEBUG)                                  */
/*         format    - format printf suivi des arguments variadiques                                                          */
/* Sortie: neant                                                                                                              */
/*                                                                                                                            */
/* Comportement de filtrage :                                                                                                 */
/*   - Si le facility figure dans Debug_facilitys -> le message est toujours emis                                             */
/*   - Sinon -> syslog filtre normalement selon setlogmask                                                                    */
/******************************************************************************************************************************/
 void Info ( const gchar *function, const gchar *facility, guint priority, const gchar *format, ... )
  { va_list ap;

    va_start ( ap, format );
    Info_full ( function, NULL, facility, priority, format, ap );
    va_end ( ap );
  }
/******************************************************************************************************************************/
/* Info_with_prefix: Envoie un message structure JSON vers syslog                                                             */
/* Entree: function  - nom de la fonction appelante (__func__)                                                                */
/*         facility   - sous-systeme / module (ex: "smsg", "json") ; NULL -> ""                                               */
/*         priority  - niveau syslog (LOG_ERR, LOG_WARNING, LOG_NOTICE, LOG_INFO, LOG_DEBUG)                                  */
/*         format    - format printf suivi des arguments variadiques                                                          */
/* Sortie: neant                                                                                                              */
/*                                                                                                                            */
/* Comportement de filtrage :                                                                                                 */
/*   - Si le facility figure dans Debug_facilitys -> le message est toujours emis                                             */
/*   - Sinon -> syslog filtre normalement selon setlogmask                                                                    */
/******************************************************************************************************************************/
 void Info_with_prefix ( const gchar *function, const gchar *facility, const gchar *prefixe, guint priority, const gchar *format, ... )
  { va_list ap;

    va_start ( ap, format );
    Info_full ( function, prefixe, facility, priority, format, ap );
    va_end ( ap );
  }
/******************************************************************************************************************************/
/* Info_change_log_level: Change le niveau de log global                                                                      */
/* Entree: le nouveau niveau (LOG_DEBUG, LOG_INFO, etc.)                                                                      */
/* Sortie: neant                                                                                                              */
/******************************************************************************************************************************/
 void Info_change_log_level ( guint new_log_level )
  { Log_level = new_log_level;
    Info ( __func__, "log", LOG_NOTICE, "Log level set to %d", new_log_level );
  }
/******************************************************************************************************************************/
/* Info_stop: Ferme la connexion syslog                                                                                       */
/* Entree: neant                                                                                                              */
/* Sortie: neant                                                                                                              */
/******************************************************************************************************************************/
 static void Info_stop ( int code_retour, void *data )
  { Info ( __func__, "log", LOG_NOTICE, "End of logs" );
    Info_clear_debug_facilities ();
    g_rw_lock_clear ( &Debug_facilities_lock );
    closelog();
  }
/******************************************************************************************************************************/
/* Info_init: Initialise le sous-systeme de log                                                                               */
/* Entree: entete    - identifiant du processus dans syslog (NULL -> nom du processus)                                        */
/*         log_level - niveau initial (LOG_DEBUG = 7, LOG_INFO = 6, ...)                                                      */
/* Sortie: neant                                                                                                              */
/******************************************************************************************************************************/
 void Info_init ( const gchar *entete, const gchar *prefixe_name, guint log_level )
  { g_rw_lock_init ( &Debug_facilities_lock );
    Debug_facilities = NULL;
    Prefixe_name     = prefixe_name;
    on_exit( Info_stop, NULL );
    openlog ( entete, LOG_CONS | LOG_PID, LOG_USER );
    Info ( __func__, "log", LOG_INFO, "Start of logs with ABLS_LIBS_VERSION=%s", ABLS_LIBS_VERSION );
    Info_change_log_level ( log_level );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/