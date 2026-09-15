/******************************************************************************************************************************/
/* include/heure.h      Declaration des prototypes de gestion temporelle — abls-libs                                          */
/* Projet Abls-Habitat version 1.0       Gestion d'habitat                                                21.07.2026          */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * heure.h
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

#ifndef _ABLS_HEURE_H_
 #define _ABLS_HEURE_H_

 #include <glib.h>
 #include <time.h>

/*-- Structure d'état pour les déclenchements temporels ----------------------------------------------------------------------*/
/*   A déclarer par l'appelant et initialisée à zéro (ex : ABLS_HEURE ctx = {};)                                              */
 struct ABLS_HEURE
  { time_t last_triggered;                                     /* horodatage (secondes depuis epoch) du dernier déclenchement */
  };

/*-- Déclenchement ponctuel à une seconde précise ----------------------------------------------------------------------------*/
 extern gboolean Heure_at_sec ( struct ABLS_HEURE *ctx, gint heure, gint minute, gint seconde );

/*-- Déclenchement ponctuel à une minute précise -----------------------------------------------------------------------------*/
 extern gboolean Heure_at_min ( struct ABLS_HEURE *ctx, gint heure, gint minute );

/*-- Déclenchement à chaque changement de minute (heure:minute:00) -----------------------------------------------------------*/
 extern gboolean Heure_every_min ( struct ABLS_HEURE *ctx );

/*-- Déclenchement toutes les 5 minutes pleines (heure:00:00, heure:05:00, ...) ----------------------------------------------*/
 extern gboolean Heure_every_5_min ( struct ABLS_HEURE *ctx );

/*-- Déclenchement toutes les 15 minutes pleines (heure:00:00, heure:15:00, ...) ---------------------------------------------*/
 extern gboolean Heure_every_15_min ( struct ABLS_HEURE *ctx );

/*-- Déclenchement à chaque heure pleine (heure:00:00) -----------------------------------------------------------------------*/
 extern gboolean Heure_every_hour ( struct ABLS_HEURE *ctx );

/*-- Déclenchement toutes les 2 heures pleines -------------------------------------------------------------------------------*/
 extern gboolean Heure_every_2_hours ( struct ABLS_HEURE *ctx );

/*-- Déclenchement unique en fin de journée (23:59:59) -----------------------------------------------------------------------*/
 extern gboolean Heure_every_end_of_day ( struct ABLS_HEURE *ctx );

/*-- Gestion des next top ----------------------------------------------------------------------------------------------------*/
 extern time_t   Top_set_next_in ( guint target_time );
 extern gboolean Top_is_out ( time_t target_time );
 extern guint    Top_remaining ( time_t target_time );

/*-- Gestion des last top ----------------------------------------------------------------------------------------------------*/
 extern time_t   Top_set_now ( void );
 extern guint    Top_since ( time_t reference_time );
 extern gboolean Top_is_too_old ( time_t reference_time, guint age );

#endif /* _ABLS_HEURE_H_ */
/*----------------------------------------------------------------------------------------------------------------------------*/
