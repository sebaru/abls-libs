/******************************************************************************************************************************/
/* src/config.c         Parsage de configuration depuis FILE, ENV, ARGV — ABLS-LIBS                                           */
/* Projet Abls-Habitat                   Gestion d'habitat                                      sam 04 jul 2026 18:28:10 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * config.c
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

 #include <string.h>
 #include <strings.h>
 #include "abls-libs.h"

/******************************************************************************************************************************/
/* Config_apply_FILE: Charge configuration depuis fichier JSON                                                                */
/* Entrée: target (JsonNode à remplir), filename (chemin du fichier)                                                          */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Config_apply_FILE ( JsonNode *target, const gchar *filename )
  { const char *name;
    JsonObjectIter iter;
    JsonNode *ObjectMemberNode;

    if (!target || !filename) return;

    Info ( __func__, FACILITY_CONFIG, NULL, LOG_NOTICE, "Trying to read config file '%s'", filename );
    JsonNode *from_file = Json_read_from_file ( (gchar *)filename );
    if (from_file)                                                              /* Copy des elements de from_file vers target */
     { JsonObject *fromFileObject = json_node_get_object(from_file);                        /* Récupération de l'objet source */
       json_object_iter_init(&iter, fromFileObject);
       while (json_object_iter_next(&iter, &name, &ObjectMemberNode))
        { Json_copy_member_into ( from_file, name, target ); }
       Json_unref( from_file );
     } else Info ( __func__, FACILITY_CONFIG, NULL, LOG_WARNING, "Unable to read file config '%s'", filename );
  }

/******************************************************************************************************************************/
/* Config_apply_ENV: Applique variables d'environnement ABLS_* dans le JSON target                                            */
/* Entrée: target (JsonNode à remplir)                                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Config_apply_ENV ( JsonNode *target )
  { gchar **env_vars;
    gchar **env;
    const gchar *valeur;
    gchar *env_name;

    if (!target) return;

    Info ( __func__, FACILITY_CONFIG, NULL, LOG_NOTICE, "Apply ENVironment Variables" );
    env_vars = g_listenv();                                               /* Récupérer la liste des variables d'environnement */
    for (env = env_vars; *env != NULL; env++)                                               /* Parcourir toutes les variables */
     { const gchar *prefixe = "ABLS_";
       if (g_str_has_prefix(*env, prefixe))                                   /* Vérifier si la variable commence par "ABLS_" */
        { valeur = g_getenv ( *env );                                                                    /* Extrait la valeur */
          if (valeur)
           { env_name = g_ascii_strdown( *env + strlen(prefixe), -1 );                                /* Passage en lowercase */
             Info ( __func__, FACILITY_CONFIG, NULL, LOG_NOTICE, "Apply ENV '%s' -> '%s' = '%s'", *env, env_name, valeur );
                  if ( !strcasecmp ( valeur, "TRUE"  ) ) { Json_add_bool ( target, env_name, TRUE ); }
             else if ( !strcasecmp ( valeur, "FALSE" ) ) { Json_add_bool ( target, env_name, FALSE ); }
             else
              { gchar *endptr = NULL;                 /* Convert only strict integers; keep values like 127.0.0.1 as strings. */
                g_ascii_strtoll ( valeur, &endptr, 10 );
                if (endptr && *endptr == '\0' && endptr != valeur)
                 { Json_add_int  ( target, env_name, atoi(valeur) ); }
                else Json_add_string ( target, env_name, valeur );                        /* Sinon d'une chaine de caracteres */
              }
             g_free(env_name);
           }
        }
     }
  }

/******************************************************************************************************************************/
/* Config_argv_callback: Callback public pour injection d'options dans JSON via GOption                                       */
/* Entrée: option_name, value, data (JsonNode *), error                                                                       */
/* Sortie: TRUE si succès, FALSE + GError si erreur                                                                           */
/* Usage: À passer comme arg_data dans GOptionEntry avec G_OPTION_ARG_CALLBACK, puis passer via user_data du GOptionGroup     */
/******************************************************************************************************************************/
 gboolean Config_argv_callback( const gchar *option_name, const gchar *value, gpointer data, GError **error )
  { JsonNode *target = data;
    const gchar *key;

    if (!target)
     { g_set_error(error, G_OPTION_ERROR, G_OPTION_ERROR_FAILED, "Invalid config target");
       return FALSE;
     }

    key = option_name;                                                            /* Extraire le nom de la clé (sans - ou --) */
    if (key[0] == '-') key++;
    if (key[0] == '-') key++;

    /* Ce callback injecte la valeur directement dans le JSON
     * L'application choisit le nom explicite de la clé dans son GOptionEntry.long_name */
    if (value)
     { if (!strcasecmp(value, "true"))                          /* Tentative de conversion en entier ou booléen, sinon string */
        { Json_add_bool(target, (gchar *)key, TRUE); }
       else if (!strcasecmp(value, "false"))
        { Json_add_bool(target, (gchar *)key, FALSE);}
       else
        { gchar *endptr = NULL;
          g_ascii_strtoll(value, &endptr, 10);
          if (endptr && *endptr == '\0' && endptr != value)
           { Json_add_int(target, (gchar *)key, atoi(value)); }
          else
           { Json_add_string(target, (gchar *)key, value); }
        }
       Info ( __func__, FACILITY_CONFIG, NULL, LOG_DEBUG, "Apply ARGV '--%s' = '%s'", key, value );
     }
    else
     { Json_add_bool(target, (gchar *)key, TRUE);                               /* Option sans argument (flag) → booléen TRUE */
       Info ( __func__, FACILITY_CONFIG, NULL, LOG_DEBUG, "Apply ARGV '--%s' (flag)", key );
     }
    return TRUE;
  }
/******************************************************************************************************************************/
/* Config_apply_ARGV: Parse argc/argv via GOptionContext                                                                      */
/* Entrée: target (JsonNode à remplir), argc/argv pointers, entries (tableau GOptionEntry)                                    */
/* Sortie: néant (GError loggué si parsing échoue)                                                                            */
/* NOTE: Pour injecter automatiquement dans JSON, les entries doivent avoir:                                                  */
/*       - arg = G_OPTION_ARG_CALLBACK                                                                                        */
/*       - arg_data = Config_argv_callback                                                                                    */
/*       - user_data du GOptionGroup doit pointer vers le JsonNode de target                                                  */
/******************************************************************************************************************************/
 void Config_apply_ARGV ( JsonNode *target, int *argc, char ***argv, GOptionEntry *entries )
  { GOptionContext *ctx;
    GOptionGroup *group;
    GError *error = NULL;

    if (!target || !argc || !argv) return;

    if (!entries)
     { Info ( __func__, FACILITY_CONFIG, NULL, LOG_DEBUG, "Config_apply_ARGV: entries is NULL, skipping ARGV parsing" );
       return;
     }

    Info ( __func__, FACILITY_CONFIG, NULL, LOG_NOTICE, "Apply Command-Line Arguments" );

    ctx = g_option_context_new("- ABLS Configuration");                                             /* Créer contexte GOption */
    group = g_option_context_get_main_group(ctx);

    g_option_group_add_entries(group, entries);                                 /* Ajouter les entries fournis par l'appelant */

    if (!g_option_context_parse(ctx, argc, argv, &error))                                                           /* Parser */
     { Info ( __func__, FACILITY_CONFIG, NULL, LOG_WARNING, "ARGV parsing failed: %s", error->message );
       g_error_free(error);
     }
    g_option_context_free(ctx);
  }

/*----------------------------------------------------------------------------------------------------------------------------*/
