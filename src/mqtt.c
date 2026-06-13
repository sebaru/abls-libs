/******************************************************************************************************************************/
/* src/mqtt.c      Couche MQTT partagee pour Abls-Libs                                                                        */
/* Projet Abls-Libs version 1.0       Gestion d'habitat                                                     13.06.2026       */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mqtt.c
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

 #include <glib.h>

 #include "mqtt.h"

/******************************************************************************************************************************/
/* abls_mqtt_default_ca_file: Recherche le fichier CA systeme par defaut pour la validation TLS MQTT                               */
/* Entrées: néant                                                                                                             */
/* Sortie : le chemin du fichier CA, ou NULL si aucun fichier n'est disponible                                                */
/******************************************************************************************************************************/
 static const gchar *abls_mqtt_default_ca_file ( void )
  { if ( g_file_test ( "/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem", G_FILE_TEST_IS_REGULAR ) )
     { return("/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem"); }

   if ( g_file_test ( "/etc/ssl/certs/ca-certificates.crt", G_FILE_TEST_IS_REGULAR ) )
     { return("/etc/ssl/certs/ca-certificates.crt"); }

   if ( g_file_test ( "/etc/ssl/certs/ca-bundle.crt", G_FILE_TEST_IS_REGULAR ) )
     { return("/etc/ssl/certs/ca-bundle.crt"); }

    return(NULL);
  }

/******************************************************************************************************************************/
/* abls_mqtt_default_ca_path: Recherche le repertoire CA systeme par defaut pour la validation TLS MQTT                        */
/* Entrées: néant                                                                                                             */
/* Sortie : le chemin du repertoire CA, ou NULL si aucun repertoire n'est disponible                                          */
/******************************************************************************************************************************/
 static const gchar *abls_mqtt_default_ca_path ( void )
  { if ( g_file_test ( "/etc/pki/tls/certs", G_FILE_TEST_IS_DIR ) )
     { return("/etc/pki/tls/certs"); }

    if ( g_file_test ( "/etc/ssl/certs", G_FILE_TEST_IS_DIR ) )
     { return("/etc/ssl/certs"); }

    return(NULL);
  }
/******************************************************************************************************************************/
/* abls_mqtt_subscribe: Abonne le client MQTT à un topic spécifique                                                          */
/* Entrées: le client MQTT, le topic à abonner                                                                               */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void abls_mqtt_subscribe( struct ABLS_MQTT *mqtt, gchar *format, ... )
  { gchar topic_full[256];
    va_list ap;
    if(!mqtt || !format) return;
    va_start( ap, format );
    g_vsnprintf ( topic_full, sizeof(topic_full), format, ap );
    va_end ( ap );
    mqtt->subscribed_topics = g_slist_append(mqtt->subscribed_topics, g_strdup(topic_full));
  }
/******************************************************************************************************************************/
/* abls_mqtt_on_connect_CB: appelé par la librairie quand le broker est connecté                                              */
/* Entrée: les parametres d'affichage de log de la librairie                                                                  */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void abls_mqtt_on_connect_CB( struct mosquitto *mosq, void *obj, int return_code )
  { struct ABLS_MQTT *mqtt = (struct ABLS_MQTT *)obj;
    abls_info( __func__, mqtt->log_facility, LOG_NOTICE, "Connected with return code %d: %s",
               return_code, mosquitto_connack_string( return_code ) );
    if (return_code == 0)
     { Partage->MQTT_connected = TRUE ;
       GSList *liste = mqtt->subscribed_topics;
       while (liste)                                                                    /* souscrit aux topics a la connexion */
        { gchar *topic = (gchar *)liste->data;
          if ( mosquitto_subscribe( mqtt->MOSQ_session, NULL, topic, mqtt->qos ) != MOSQ_ERR_SUCCESS )
           { abls_info ( Info_new( __func__, mqtt->log_facility, LOG_ERR, "Subscribe to topic '%s' FAILED", topic ); }
          else
           { abls_info ( Info_new( __func__, mqtt->log_facility, LOG_INFO, "Subscribe to topic '%s' OK", topic ); }
          liste = liste->next;
        }
      while ( g_async_queue_length ( mqtt->queue ) )                               /* s'il y en a, envoie les messages queued */
        { JsonNode *node = g_async_queue_pop ( mqtt->queue );
          gchar *topic    = Json_get_string ( node, "mqtt_queued_topic" );
          gboolean retain = Json_get_boolean ( node, "mqtt_queued_retain" );
          gchar *buffer = Json_node_to_string ( node );
          if (buffer)
           { mosquitto_publish( mqtt->MOSQ_session, NULL, topic, strlen(buffer), buffer, mqtt->qos, retain );
             g_free(buffer);
           }
          Json_node_free(node);
        }
     }
  }
/******************************************************************************************************************************/
/* abls_mqtt_on_disconnect_CB: appelé par la librairie quand le broker est déconnecté                                         */
/* Entrée: les parametres d'affichage de log de la librairie                                                                  */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void abls_mqtt_on_disconnect_CB( struct mosquitto *mosq, void *obj, int return_code )
  { struct ABLS_MQTT *mqtt = (struct ABLS_MQTT *)obj;
    abls_info_new( __func__, mqtt->log_facility, LOG_NOTICE, "Disconnected with return code %d: %s",
                   return_code, mosquitto_connack_string( return_code ) );
    Partage->MQTT_connected = FALSE;
  }
/******************************************************************************************************************************/
/* abls_mqtt_on_message_CB: Appelé par mosquitto lorsqu'un message MQTT est reçu                                              */
/* Entrée: session MQTT, contexte utilisateur, message reçu                                                                    */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void abls_mqtt_on_message_CB ( struct mosquitto *MOSQ_session, void *obj, const struct mosquitto_message *msg )
  { gchar **tokens = g_strsplit ( msg->topic, "/", 3 );
    if (!tokens) return;
    if (!tokens[0]) goto end; /* Normalement le domain_uuid  */
    if (!tokens[1]) goto end; /* Normalement le agent_uuid, ou le tag */
    if (!tokens[2]) goto end; /* Normalement l'operation  */

    if ( strcasecmp( tokens[0], Json_get_string ( Config.config, "domain_uuid" ) ) )
     { abls_info_new( __func__, Config.log_msrv, LOG_NOTICE, "Wrong domain_uuid '%s'. Dropping.", tokens[0] ); goto end; }

    abls_info_new( __func__, Config.log_msrv, LOG_DEBUG, "MQTT Message received from API: %s/%s", tokens[1], tokens[2] );

/*-------------------------------------------------- Message with payload ----------------------------------------------------*/
    JsonNode *request = NULL;                                      /* Request peut etre nulle si mal formée ou pas de payload */
    if (msg->payload)
     { request = Json_get_from_string ( msg->payload ); }          /* Request peut etre nulle si mal formée ou pas de payload */

/*-------------------------------------------------- topic Agent -------------------------------------------------------------*/
    if ( !strcasecmp( tokens[1], Json_get_string ( Config.config, "agent_uuid" ) ) )
     { if ( !strcasecmp( tokens[2], "SET") )
        { if ( !( Json_has_member ( request, "log_bus" ) && Json_has_member ( request, "log_level" ) &&
                  Json_has_member ( request, "log_dls" ) &&
                  Json_has_member ( request, "log_msrv" ) && Json_has_member ( request, "headless" )
                )
             )
           { abls_info_new( __func__, Config.log_msrv, LOG_ERR, "AGENT_SET: wrong parameters" );
             goto end;
           }
          Config.log_bus    = Json_get_bool ( request, "log_bus" );
          Config.log_msrv   = Json_get_bool ( request, "log_msrv" );
          Config.log_dls    = Json_get_bool ( request, "log_dls" );
          gboolean headless = Json_get_bool ( request, "headless" );
          gint log_level    = Json_get_int  ( request, "log_level" );
          gchar *branche    = Json_get_string ( request, "branche" );
          abls_info_change_log_level ( log_level );
          abls_info_new( __func__, TRUE, LOG_NOTICE, "AGENT_SET: log_msrv=%d, log_bus=%d, log_dls=%d, log_level=%d, headless=%d",
                    Config.log_msrv, Config.log_bus, Config.log_dls, log_level, headless );
          if (Config.headless != headless)
           { abls_info_new( __func__, Config.log_msrv, LOG_NOTICE, "AGENT_SET: headless has changed, rebooting" );
             Partage->Thread_run = FALSE;
           }
          if (strcmp ( WTD_BRANCHE, branche ))
           { abls_info_new( __func__, Config.log_msrv, LOG_NOTICE, "AGENT_SET: branche has changed, upgrading and rebooting" );
             MSRV_Agent_upgrade_to ( branche );
           }
        }
       else if ( !strcasecmp( tokens[2], "RESET") )
        { abls_info_new( __func__, Config.log_msrv, LOG_NOTICE, "RESET: Stopping in progress" );
          Partage->Thread_run = FALSE;
          goto end;
        }
       else if ( !strcasecmp( tokens[2], "UPGRADE") )
        { abls_info_new( __func__, Config.log_msrv, LOG_NOTICE, "UPGRADE: Upgrading in progress" );
          MSRV_Agent_upgrade_to ( WTD_BRANCHE );
          goto end;
        }
     }
/*-------------------------------------------------- topic Thread pour le master et les slaves -------------------------------*/
    else if ( !strcasecmp( tokens[1], "THREAD") )
     {      if ( !strcasecmp( tokens[2], "STOP") )
             { Thread_Stop_by_thread_tech_id ( Json_get_string ( request, "thread_tech_id" ) ); }
       else if ( !strcasecmp( tokens[2], "START") )
             { Thread_Start_by_thread_tech_id ( Json_get_string ( request, "thread_tech_id" ) ); }
       else if ( !strcasecmp( tokens[2], "RESTART") )
             { Thread_Restart ( Json_get_string ( request, "thread_classe" ), Json_get_string ( request, "thread_tech_id" ) ); }
       else if ( Config.instance_is_master && !strcasecmp( tokens[2], "TEST") )
             { MQTT_Send_to_topic ( Partage->MQTT_local_session, request, FALSE, "SET_TEST/%s", Json_get_string ( request, "thread_tech_id" ) ); }
       else if ( Config.instance_is_master && !strcasecmp( tokens[2], "DEBUG") )
             { MQTT_Send_to_topic ( Partage->MQTT_local_session, request, FALSE, "SET_DEBUG/%s", Json_get_string ( request, "thread_tech_id" ) ); }
     }
/*-------------------------------------------------- topic DLS ---------------------------------------------------------------*/
    else if ( !strcasecmp( tokens[1], "DLS") )
     { if ( !strcasecmp( tokens[2], "RELOAD") )
        { if ( !Json_has_member ( request, "tech_id" ) )
           { abls_info_new( __func__, Config.log_msrv, LOG_ERR, "DLS_RELOAD: tech_id is missing" );
             goto end_request;
           }
          gchar *target_tech_id = Json_get_string ( request, "tech_id" );
          struct DLS_PLUGIN *found = Dls_get_plugin_by_tech_id ( target_tech_id );
          if (found) Dls_Save_Data_to_API ( found );     /* Si trouvé, on sauve les valeurs des bits internes avant rechargement */
          struct DLS_PLUGIN *dls = Dls_Importer_un_plugin ( target_tech_id );
          if (dls) abls_info_new( __func__, Config.log_dls, LOG_NOTICE, "'%s': imported", target_tech_id );
              else abls_info_new( __func__, Config.log_dls, LOG_ERR, "'%s': error when importing", target_tech_id );
          Dls_Load_horloge_ticks();
        }
       else if ( !strcasecmp( tokens[2], "SET") )
        { if ( ! Json_has_member ( request, "tech_id" )  )
           { abls_info_new( __func__, Config.log_msrv, LOG_ERR, "DLS_SET: wrong parameters" );
             goto end_request;
           }
          gchar *plugin_tech_id = Json_get_string ( request, "tech_id" );
          pthread_mutex_lock( &Partage->com_dls.synchro );               /* On stoppe DLS pour éviter la compilation multiple */
          if (Json_has_member ( request, "debug"  )) Dls_Debug_plugin   ( plugin_tech_id, Json_get_bool ( request, "debug" ) );
          if (Json_has_member ( request, "enable" )) Dls_Activer_plugin ( plugin_tech_id, Json_get_bool ( request, "enable" ) );
          pthread_mutex_unlock( &Partage->com_dls.synchro );             /* On stoppe DLS pour éviter la compilation multiple */
        }
       else if ( !strcasecmp( tokens[2], "RESTART") )
        { if ( !Json_has_member ( request, "tech_id" ) )
           { abls_info_new( __func__, Config.log_msrv, LOG_ERR, "DLS_RESTART: tech_id is missing" );
             goto end_request;
           }
          gchar *target_tech_id = Json_get_string ( request, "tech_id" );
          struct DLS_PLUGIN *plugin = Dls_get_plugin_by_tech_id ( target_tech_id );
          if (plugin)
           { plugin->vars.resetted = TRUE;                                         /* au chargement, le bit de start vaut 1 ! */
             abls_info_new( __func__, Config.log_dls, LOG_NOTICE, "'%s': _START sent to plugin", target_tech_id );
           }
          else abls_info_new( __func__, Config.log_dls, LOG_ERR, "'%s': error when resetting: plugin not found.", target_tech_id );
        }
       else if ( !strcasecmp( tokens[2], "ACQUIT") )
        { if ( !Json_has_member ( request, "tech_id" ) )
           { abls_info_new( __func__, Config.log_msrv, LOG_ERR, "DLS_ACQUIT: tech_id is missing" );
             goto end_request;
           }
          gchar *plugin_tech_id = Json_get_string ( request, "tech_id" );
          Dls_Acquitter_plugin ( plugin_tech_id );
        }
       else if ( !strcasecmp( tokens[2], "REMAP") )
        { MSRV_Remap();
          MQTT_Send_to_topic ( Partage->MQTT_local_session, NULL, FALSE, "SYNC_INPUT" );/* Synchronisation des IO depuis les threads */
          pthread_mutex_lock( &Partage->com_dls.synchro );                               /* Zone de protection des bits internes */
          Dls_foreach_plugins ( NULL, Dls_sync_all_output );                                             /* Run all plugin D.L.S */
          pthread_mutex_unlock( &Partage->com_dls.synchro );                      /* Fin de Zone de protection des bits internes */
        }
       else if ( !strcasecmp( tokens[2], "RELOAD_HORLOGE_TICK") ) Dls_Load_horloge_ticks();
     }
/*-------------------------------------------------- topic Audio Zone --------------------------------------------------------*/
    else if ( !strcasecmp( tokens[1], "AUDIO_ZONE") )
     { if (!strcasecmp ( tokens[2], "TEST" ))
        { gchar libelle_audio[256];
          gchar *audio_zone_name = Json_get_string ( request, "audio_zone_name" );
          g_snprintf ( libelle_audio, sizeof(libelle_audio), "Test de diffusion audio sur la zone '%s'", audio_zone_name );
          AUDIO_Send_to_zone ( audio_zone_name, libelle_audio );
        }
     }
/*-------------------------------------------------- topic Audio Zone --------------------------------------------------------*/
    else if ( !strcasecmp( tokens[1], "SYNOPTIQUE") )
     { if ( !strcasecmp( tokens[2], "CLIC") )
        { if ( !Json_has_member ( request, "tech_id" ) )
           { abls_info_new( __func__, Config.log_msrv, LOG_ERR, "SYN_CLIC: tech_id is missing" ); goto end_request; }
          if ( !Json_has_member ( request, "acronyme" ) )
           { abls_info_new( __func__, Config.log_msrv, LOG_ERR, "SYN_CLIC: acronyme is missing" ); goto end_request; }
          gchar *tech_id  = Json_get_string ( request, "tech_id" );
          gchar *acronyme = Json_get_string ( request, "acronyme" );
          struct DLS_DI *bit = Dls_data_DI_lookup ( tech_id, acronyme );
          if (!bit) abls_info_new( __func__, Config.log_msrv, LOG_ERR, "SYN_CLIC: '%s:%s' not found. Dropping.", tech_id, acronyme );
          else Dls_data_DI_set_pulse ( NULL, bit );
        }
     }
/*-------------------------------------------------- topic SET_GPS -----------------------------------------------------------*/
    else if ( !strcasecmp( tokens[1], "SET_GPS") )
     { if ( !Json_has_member ( request, "email" ) || !Json_has_member ( request, "latitude" ) ||
            !Json_has_member ( request, "longitude" ) )
        { abls_info_new( __func__, Config.log_msrv, LOG_ERR, "SET_GPS: missing fields. Dropping." );
          goto end_request;
        }
       gchar *email = Json_get_string ( request, "email" );
       JsonNode *user_gps = Json_node_add_objet ( Partage->Users_GPS, email );
       if (user_gps)
        { gdouble latitude = Json_get_double ( request, "latitude" );
          gdouble longitude = Json_get_double ( request, "longitude" );
          Json_node_add_double ( user_gps, "latitude",  latitude );
          Json_node_add_double ( user_gps, "longitude", longitude );
          abls_info_new( __func__, Config.log_msrv, LOG_INFO, 
                    "SET_GPS: Updated GPS for user '%s': latitude=%f, longitude=%f", email, latitude, longitude );
        }
     }

end_request:
    Json_node_unref ( request );
end:
    g_strfreev( tokens );                                                                      /* Libération des tokens topic */
  }
/******************************************************************************************************************************/
/* abls_mqtt_send: Envoie le node au broker                                                                                   */
/* Entrée: le topic, le node                                                                                                  */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void abls_mqtt_send ( struct ABLS_MQTT *mqtt, JsonNode *node, gboolean retain, gchar *topic, ... )
  { gchar topic_inter[256], topic_full[256];
    va_list ap;
    if (!topic) return;

    va_start( ap, topic );
    g_vsnprintf ( topic_full, sizeof(topic_full), topic, ap );
    va_end ( ap );

    gboolean free_node=FALSE;
    if (!node) { node = Json_node_create(); free_node = TRUE; }
    gchar *buffer = Json_node_to_string ( node );

    gint retour = mosquitto_publish( mqtt->MOSQ_session, NULL, topic_full, strlen(buffer), buffer, mqtt->qos, retain );
    if (retour != MOSQ_ERR_SUCCESS )                               /* Si problème d'envoi au MQTT Broker, on garde en mémoire */
     { abls_info( __func__, mqtt->log_facility, LOG_ERR, "MQTT publish error: %s. Keeping in memory to send later.", mosquitto_strerror ( retour ) );
       JsonNode *new_node = Json_node_copy ( node );                      /* Prépare un new node avec topic et retain intégré */
       Json_node_add_string  ( new_node, "mqtt_queued_topic", topic_full );
       Json_node_add_boolean ( new_node, "mqtt_queued_retain", retain );
       g_async_queue_push ( mqtt->queue, new_node );
     }
    g_free(buffer);
    if (free_node) Json_node_unref(node);
  }
/******************************************************************************************************************************/
/* abls_mqtt_init: Alloue/initialise un client MQTT et configure les callbacks                                                */
/* Entrées: paramètres de connexion, sécurité TLS et journalisation                                                            */
/* Sortie: pointeur ABLS_MQTT prêt à démarrer, ou NULL si erreur                                                               */
/******************************************************************************************************************************/
 struct ABLS_MQTT *abls_mqtt_init ( const gchar *log_facility, const gchar *client_id,
                                    gboolean is_ssl, const gchar *ca_file, const gchar *ca_path,
                                    const gchar *username, const gchar *password, const gchar *hostname, gint port, gint qos )
  { gint retour;

    if (!log_facility) log_facility = "mqtt";

    if (is_ssl && !ca_file && !ca_path)
     { abls_info( __func__, log_facility, LOG_ERR, "MQTT TLS setup error: no CA file or CA path found." );
       return(NULL);
     }

    struct ABLS_MQTT *mqtt = g_try_malloc0 ( sizeof ( struct ABLS_MQTT ) );
    if (!mqtt)
     { abls_info( __func__, log_facility, LOG_ERR, "Memory error." ); return(NULL); }

    mqtt->connected = FALSE;
    mqtt->qos       = qos;
    mqtt->queue     = g_async_queue_new_full( g_free );
    mqtt->subscribed_topics = NULL;
    g_strlcpy(mqtt->log_facility, log_facility, sizeof(mqtt->log_facility));

    mqtt->MOSQ_session = mosquitto_new( client_id, TRUE, mqtt );
    if (!mqtt->MOSQ_session)
     { abls_info( __func__, mqtt->log_facility, LOG_ERR, "Cannot create mosquitto session." );
       abls_mqtt_stop (mqtt);
       return(NULL);
     }

    mosquitto_log_callback_set        ( mqtt->MOSQ_session, abls_mqtt_on_log_CB );
    mosquitto_connect_callback_set    ( mqtt->MOSQ_session, abls_mqtt_on_connect_CB );
    mosquitto_disconnect_callback_set ( mqtt->MOSQ_session, abls_mqtt_on_disconnect_CB );
    mosquitto_message_callback_set    ( mqtt->MOSQ_session, abls_mqtt_on_message_CB );
    mosquitto_reconnect_delay_set     ( mqtt->MOSQ_session, 10, 60, TRUE );

    if (is_ssl)
     { gint retour_tls = mosquitto_tls_set( mqtt->MOSQ_session, ca_file, ca_path, NULL, NULL, NULL );
       if ( retour_tls != MOSQ_ERR_SUCCESS )
        { abls_info( __func__, mqtt->log_facility, LOG_ERR, "MQTT TLS setup error: %s", mosquitto_strerror(retour_tls) );
          abls_mqtt_stop (mqtt);
          return(NULL);
        }
       abls_info( __func__, mqtt->log_facility, LOG_INFO, "MQTT TLS trust store: cafile='%s', capath='%s'",
                  ca_file ? ca_file : "", ca_path ? ca_path : "" );
     }

    if (username)
     { mosquitto_username_pw_set( mqtt->MOSQ_session, username, password ); }
    return(mqtt);
  }
/******************************************************************************************************************************/
/* abls_mqtt_start: Lance la connexion réseau MQTT et la boucle asynchrone                                                    */
/* Entrée: client MQTT initialisé                                                                                              */
/* Sortie: TRUE si démarrage réussi, FALSE sinon                                                                               */
/******************************************************************************************************************************/
 gboolean abls_mqtt_start ( struct ABLS_MQTT *mqtt )
  { if (!mqtt) return(FALSE);
    gboolean retour = mosquitto_connect( mqtt->MOSQ_session, hostname, port, 60 );
    if ( retour != MOSQ_ERR_SUCCESS )
     { abls_info( __func__, mqtt->log_facility, LOG_ERR, "MQTT_API connection to '%s' error: %s",
                 hostname, mosquitto_strerror ( retour ) );
       return(FALSE);}

    retour = mosquitto_loop_start( mqtt->MOSQ_session );
    if ( retour != MOSQ_ERR_SUCCESS )
     { abls_info( __func__, mqtt->log_facility, LOG_ERR, "MQTT loop not started: %s", mosquitto_strerror ( retour ) );
       return(FALSE);
     }
    abls_info( __func__, mqtt->log_facility, LOG_NOTICE, "Connected as %s with id %s on %s:%d.",
               username, client_id, hostname, port );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* abls_mqtt_stop: Arrête la session MQTT et libère toutes les ressources associées                                           */
/* Entrée: client MQTT                                                                                                         */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void abls_mqtt_stop ( struct ABLS_MQTT *mqtt )
  { mosquitto_disconnect( mqtt->MOSQ_session );
    mosquitto_loop_stop( mqtt->MOSQ_session, FALSE );
    mosquitto_destroy( mqtt->MOSQ_session );
    mqtt->MOSQ_session = NULL;
    if (mqtt->queue) g_async_queue_unref(mqtt->queue);
    if (mqtt->subscribed_topics) g_slist_free_full(mqtt->subscribed_topics, g_free);
    g_free(mqtt);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
