/******************************************************************************************************************************/
/* include/info.h      Declaration des prototypes de gestion de log abls_info — abls-libs                                   */
/* Projet Abls-Habitat version 1.0       Gestion d'habitat                                                13.06.2026          */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * info.h
 * This file is part of Abls-Habitat
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

#ifndef _ABLS_INFO_H_
 #define _ABLS_INFO_H_

 #include <glib.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <syslog.h>
 #include <string.h>
 #include <sys/prctl.h>
 #include <stdarg.h>

/*-- Initialisation / arret --------------------------------------------------------------------------------------------------*/
 extern void abls_info_init             ( const gchar *entete, const gchar *prefixe_name, guint log_level );
 extern void abls_info_change_log_level ( guint new_log_level );

/*-- Logging with prefixe ----------------------------------------------------------------------------------------------------*/
 extern void abls_info ( const gchar *function,
                         const gchar *facility,
                         guint        priority,
                         const gchar *format, ... );

/*-- Logging with prefixe ----------------------------------------------------------------------------------------------------*/
 extern void abls_info_with_prefix ( const gchar *function,
                                     const gchar *facility,
                                     const gchar *prefixe,
                                     guint        priority,
                                     const gchar *format, ... );

                                     /*-- Compteur de messages ----------------------------------------------------------------------------------------------------*/
 extern gint abls_info_reset_nbr_log ( void );

/*-- Forcage debug par contexte ----------------------------------------------------------------------------------------------*/
 extern void abls_info_debug_facility        ( const gchar *prefixe, const gchar *context );
 extern void abls_info_undebug_facility      ( const gchar *prefixe, const gchar *context );
 extern void abls_info_clear_debug_facilities ( void );

#endif /* _ABLS_INFO_H_ */
/*----------------------------------------------------------------------------------------------------------------------------*/