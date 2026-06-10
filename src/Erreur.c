/******************************************************************************************************************************/
/* src/Erreur.c      Gestion des logs systemes — abls-habitat-libs                                                            */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                10.06.2026          */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Erreur.c
 * This file is part of Abls-Habitat
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
 static gint      Nbr_log_par_min  = 0;
 static GHashTable *Forced_contexts = NULL;
 static GMutex     Forced_mutex;

/******************************************************************************************************************************/
/* Info_force_debug: Active le forçage de debug pour un contexte donné                                                        */
/* Entrée: le nom du contexte (ex: "smsg", "bus", "json")                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Info_force_debug ( const gchar *context )
  { if (!context) return;
    g_mutex_lock ( &Forced_mutex );
    if (!Forced_contexts)
      Forced_contexts = g_hash_table_new_full ( g_str_hash, g_str_equal, g_free, NULL );
    g_hash_table_insert ( Forced_contexts, g_strdup(context), GINT_TO_POINTER(1) );
    g_mutex_unlock ( &Forced_mutex );
  }
/******************************************************************************************************************************/
/* Info_unforce_debug: Désactive le forçage de debug pour un contexte donné                                                   */
/* Entrée: le nom du contexte                                                                                                  */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Info_unforce_debug ( const gchar *context )
  { if (!context) return;
    g_mutex_lock ( &Forced_mutex );
    if (Forced_contexts) g_hash_table_remove ( Forced_contexts, context );
    g_mutex_unlock ( &Forced_mutex );
  }
/******************************************************************************************************************************/
/* Info_clear_forced_debug: Vide la liste de tous les contextes forcés                                                        */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Info_clear_forced_debug ( void )
  { g_mutex_lock ( &Forced_mutex );
    if (Forced_contexts) g_hash_table_remove_all ( Forced_contexts );
    g_mutex_unlock ( &Forced_mutex );
  }
/******************************************************************************************************************************/
/* Info_reset_nbr_log: Retourne et remet à zéro le compteur de messages de log                                                */
/* Entrée: néant                                                                                                              */
/* Sortie: le nombre de messages envoyés depuis le dernier appel                                                              */
/******************************************************************************************************************************/
 gint Info_reset_nbr_log ( void )
  { return ( g_atomic_int_and ( &Nbr_log_par_min, 0 ) );
  }
/******************************************************************************************************************************/
/* Info_new: Envoie un message structuré JSON vers syslog                                                                     */
/* Entrée: function  — nom de la fonction appelante (__func__)                                                                */
/*         context   — sous-système / module (ex: "smsg", "json") ; NULL → ""                                                */
/*         priority  — niveau syslog (LOG_ERR, LOG_WARNING, LOG_NOTICE, LOG_INFO, LOG_DEBUG)                                  */
/*         domain_uuid — UUID du domaine applicatif ; NULL → "master"                                                         */
/*         format    — format printf suivi des arguments variadiques                                                          */
/* Sortie: néant                                                                                                              */
/*                                                                                                                            */
/* Comportement de filtrage :                                                                                                 */
/*   - Si le contexte figure dans Forced_contexts → le message est toujours émis (bypass setlogmask)                         */
/*   - Sinon → syslog filtre normalement selon setlogmask                                                                     */
/******************************************************************************************************************************/
 void Info_new ( const gchar *function, const gchar *context, guint priority,
                 const gchar *domain_uuid, const gchar *format, ... )
  { gchar chaine[512], nom_thread[32];
    va_list ap;
    gboolean forced = FALSE;

    if (!function || !format) return;

    if (context)
     { g_mutex_lock ( &Forced_mutex );
       if (Forced_contexts)
         forced = g_hash_table_contains ( Forced_contexts, context );
       g_mutex_unlock ( &Forced_mutex );
     }

    prctl ( PR_GET_NAME, &nom_thread, 0, 0, 0 );

    g_snprintf ( chaine, sizeof(chaine),
                 "{ \"domain_uuid\": \"%s\", \"context\":\"%s\", \"thread\":\"%s\", \"function\":\"%s\", \"message\":\"%s\" }",
                 (domain_uuid ? domain_uuid : "master"),
                 (context     ? context     : ""),
                 nom_thread,
                 function,
                 format );

    va_start ( ap, format );
    if (forced)
     { /* Bypass setlogmask : on force l'émission en passant par LOG_EMERG momentanément
          au niveau de la priorité réelle, en ouvrant directement le socket syslog.
          La méthode portable est de changer temporairement le masque. */
       int old_mask = setlogmask ( LOG_UPTO(LOG_DEBUG) );
       vsyslog ( priority, chaine, ap );
       setlogmask ( old_mask );
     }
    else
     { vsyslog ( priority, chaine, ap ); }
    va_end ( ap );

    g_atomic_int_inc ( &Nbr_log_par_min );
  }
/******************************************************************************************************************************/
/* Info_change_log_level: Change le niveau de log global                                                                      */
/* Entrée: Le nouveau niveau (LOG_DEBUG, LOG_INFO, etc.)                                                                      */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Info_change_log_level ( guint new_log_level )
  { setlogmask ( LOG_UPTO(new_log_level) );
    Info_new ( __func__, "log", LOG_INFO, "master", "Log level set to %d", new_log_level );
  }
/******************************************************************************************************************************/
/* Info_stop: Ferme la connexion syslog                                                                                       */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Info_stop ( void )
  { Info_new ( __func__, "log", LOG_INFO, "master", "End of logs" );
    g_mutex_lock ( &Forced_mutex );
    g_clear_pointer ( &Forced_contexts, g_hash_table_destroy );
    g_mutex_unlock ( &Forced_mutex );
    closelog();
  }
/******************************************************************************************************************************/
/* Info_init: Initialise le sous-système de log                                                                               */
/* Entrée: ident     — identifiant du processus dans syslog (NULL → nom du processus)                                         */
/*         log_level — niveau initial (LOG_DEBUG = 7, LOG_INFO = 6, …)                                                       */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Info_init ( const gchar *ident, guint log_level )
  { g_mutex_init ( &Forced_mutex );
    Forced_contexts = g_hash_table_new_full ( g_str_hash, g_str_equal, g_free, NULL );
    openlog ( ident, LOG_CONS | LOG_PID, LOG_USER );
    Info_change_log_level ( log_level );
    Info_new ( __func__, "log", LOG_INFO, "master", "Start of logs" );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
