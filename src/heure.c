/******************************************************************************************************************************/
/* src/heure.c          Gestion des déclenchements temporels ponctuels — abls-libs                                            */
/* Projet Abls-Habitat version 1.0       Gestion d'habitat                                                21.07.2026          */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * heure.c
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

 #include "abls-libs.h"

/******************************************************************************************************************************/
/* Heure_at_sec: Renvoie TRUE une et une seule fois lors de la seconde heure:minute:seconde indiquée                         */
/* Entree: ctx     - pointeur vers la structure d'état de l'appelant (doit être initialisée à zéro au départ)                */
/*         heure   - heure souhaitée (0-23)                                                                                   */
/*         minute  - minute souhaitée (0-59)                                                                                  */
/*         seconde - seconde souhaitée (0-59)                                                                                 */
/* Sortie: TRUE lors du premier appel correspondant à la seconde visée ; FALSE le reste du temps                              */
/******************************************************************************************************************************/
 gboolean Heure_at_sec ( HEURE_AT *ctx, gint heure, gint minute, gint seconde )
  { struct tm tm_now;
    time_t now;

    if (!ctx) return (FALSE);
    now = time (NULL);
    localtime_r ( &now, &tm_now );

    if (tm_now.tm_hour != heure || tm_now.tm_min != minute || tm_now.tm_sec != seconde) return (FALSE);
    if (ctx->last_triggered == now) return (FALSE);

    ctx->last_triggered = now;
    return (TRUE);
  }
/******************************************************************************************************************************/
/* Heure_at_min: Renvoie TRUE une et une seule fois lors de la minute heure:minute indiquée                                  */
/* Entree: ctx    - pointeur vers la structure d'état de l'appelant (doit être initialisée à zéro au départ)                 */
/*         heure  - heure souhaitée (0-23)                                                                                    */
/*         minute - minute souhaitée (0-59)                                                                                   */
/* Sortie: TRUE lors du premier appel correspondant à la minute visée ; FALSE le reste du temps                               */
/******************************************************************************************************************************/
 gboolean Heure_at_min ( HEURE_AT *ctx, gint heure, gint minute )
  { struct tm tm_now;
    time_t now, minute_start;

    if (!ctx) return (FALSE);
    now = time (NULL);
    localtime_r ( &now, &tm_now );

    if (tm_now.tm_hour != heure || tm_now.tm_min != minute) return (FALSE);
    minute_start = now - tm_now.tm_sec;
    if (ctx->last_triggered == minute_start) return (FALSE);

    ctx->last_triggered = minute_start;
    return (TRUE);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
