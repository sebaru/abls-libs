/******************************************************************************************************************************/
/* include/config.h    Configuration helpers pour ABLS-LIBS — parsage FILE, ENV, ARGV                                       */
/* Projet Abls-Habitat version 1.0       Gestion d'habitat                                                14.06.2026          */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * config.h
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

#ifndef _ABLS_CONFIG_H_
 #define _ABLS_CONFIG_H_

 #include <glib.h>
 #include <json-glib/json-glib.h>

 #define FACILITY_CONFIG "config"

/*-- Configuration parsers ---------------------------------------------------------------------------------------------------------------*/

/* Contexte pour callback GOption → JSON */
typedef struct {
    JsonNode *json;
} ConfigArgvCtx;

 extern gboolean Config_argv_callback( const gchar *option_name, const gchar *value, gpointer data, GError **error);
 extern void Config_add_parameter ( const gchar *name, const gchar *description );
 extern void Config_apply_FILE  ( JsonNode *target, const gchar *filename );
 extern void Config_apply_ENV   ( JsonNode *target );
 extern void Config_apply_ARGV  ( JsonNode *target, gint argc, gchar **argv );

#endif /* _ABLS_CONFIG_H_ */
/*----------------------------------------------------------------------------------------------------------------------------*/
