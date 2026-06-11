/******************************************************************************************************************************/
/* include/Erreur.h      Déclaration des prototypes de gestion de log — abls-habitat-libs                                     */
/* Projet Abls-Habitat version 1.0       Gestion d'habitat                                                10.06.2026          */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Erreur.h
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

#ifndef _ERREUR_H_
 #define _ERREUR_H_

 #include <glib.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <syslog.h>
 #include <string.h>
 #include <sys/prctl.h>
 #include <stdarg.h>

/*-- Initialisation / arrêt --------------------------------------------------*/
 extern void Info_init             ( const gchar *ident, guint log_level );
 extern void Info_change_log_level ( guint new_log_level );

/*-- Logging principal -------------------------------------------------------*/
 extern void Info_new ( const gchar *function,
                        const gchar *context,
                        guint        priority,
                        const gchar *domain_uuid,
                        const gchar *format, ... );

/*-- Compteur de messages ----------------------------------------------------*/
 extern gint Info_reset_nbr_log ( void );

/*-- Forçage debug par contexte ----------------------------------------------*/
 extern void Info_debug_context        ( const gchar *context );
 extern void Info_undebug_context      ( const gchar *context );
 extern void Info_clear_debug_contexts ( void );

#endif /* _ERREUR_H_ */
/*----------------------------------------------------------------------------------------------------------------------------*/
