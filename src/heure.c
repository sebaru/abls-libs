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
 gboolean Heure_at_sec ( ABLS_HEURE *ctx, gint heure, gint minute, gint seconde )
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
/* Heure_at_min: Renvoie TRUE une et une seule fois lors de la minute heure:minute indiquée                                   */
/* Entree: ctx    - pointeur vers la structure d'état de l'appelant (doit être initialisée à zéro au départ)                  */
/*         heure  - heure souhaitée (0-23)                                                                                    */
/*         minute - minute souhaitée (0-59)                                                                                   */
/* Sortie: TRUE lors du premier appel correspondant à la minute visée ; FALSE le reste du temps                               */
/******************************************************************************************************************************/
 gboolean Heure_at_min ( ABLS_HEURE *ctx, gint heure, gint minute )
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
/******************************************************************************************************************************/
/* Heure_every_min: Renvoie TRUE une et une seule fois au changement de minute, à la seconde 00                               */
/* Entree: ctx - pointeur vers la structure d'état de l'appelant (doit être initialisée à zéro au départ)                     */
/* Sortie: TRUE lors du premier appel correspondant à l'instant heure:minute:00 ; FALSE le reste du temps                     */
/******************************************************************************************************************************/
 gboolean Heure_every_min ( ABLS_HEURE *ctx )
  { struct tm tm_now;
    time_t now, minute_start;

    if (!ctx) return (FALSE);
    now = time (NULL);
    localtime_r ( &now, &tm_now );

    if (tm_now.tm_sec != 0) return (FALSE);
    minute_start = now;
    if (ctx->last_triggered == minute_start) return (FALSE);

    ctx->last_triggered = minute_start;
    return (TRUE);
  }
/******************************************************************************************************************************/
/* Heure_every_5_min: Renvoie TRUE une et une seule fois toutes les 5 minutes pleines                                        */
/* Entree: ctx - pointeur vers la structure d'état de l'appelant (doit être initialisée à zéro au départ)                   */
/* Sortie: TRUE lors du premier appel correspondant à l'instant heure:00|05|10...:00 ; FALSE le reste du temps              */
/******************************************************************************************************************************/
 gboolean Heure_every_5_min ( ABLS_HEURE *ctx )
  { struct tm tm_now;
    time_t now, minute_start;

    if (!ctx) return (FALSE);
    now = time (NULL);
    localtime_r ( &now, &tm_now );

    if (tm_now.tm_sec != 0 || (tm_now.tm_min % 5) != 0) return (FALSE);
    minute_start = now;
    if (ctx->last_triggered == minute_start) return (FALSE);

    ctx->last_triggered = minute_start;
    return (TRUE);
  }
/******************************************************************************************************************************/
/* Heure_every_15_min: Renvoie TRUE une et une seule fois toutes les 15 minutes pleines                                      */
/* Entree: ctx - pointeur vers la structure d'état de l'appelant (doit être initialisée à zéro au départ)                   */
/* Sortie: TRUE lors du premier appel correspondant à l'instant heure:00|15|30|45:00 ; FALSE le reste du temps              */
/******************************************************************************************************************************/
 gboolean Heure_every_15_min ( ABLS_HEURE *ctx )
  { struct tm tm_now;
    time_t now, minute_start;

    if (!ctx) return (FALSE);
    now = time (NULL);
    localtime_r ( &now, &tm_now );

    if (tm_now.tm_sec != 0 || (tm_now.tm_min % 15) != 0) return (FALSE);
    minute_start = now;
    if (ctx->last_triggered == minute_start) return (FALSE);

    ctx->last_triggered = minute_start;
    return (TRUE);
  }
/******************************************************************************************************************************/
/* Heure_every_hour: Renvoie TRUE une et une seule fois à chaque heure pleine, à la seconde 00                                */
/* Entree: ctx - pointeur vers la structure d'état de l'appelant (doit être initialisée à zéro au départ)                     */
/* Sortie: TRUE lors du premier appel correspondant à l'instant heure:00:00 ; FALSE le reste du temps                         */
/******************************************************************************************************************************/
 gboolean Heure_every_hour ( ABLS_HEURE *ctx )
  { struct tm tm_now;
    time_t now, hour_start;

    if (!ctx) return (FALSE);
    now = time (NULL);
    localtime_r ( &now, &tm_now );

    if (tm_now.tm_min != 0 || tm_now.tm_sec != 0) return (FALSE);
    hour_start = now;
    if (ctx->last_triggered == hour_start) return (FALSE);

    ctx->last_triggered = hour_start;
    return (TRUE);
  }
/******************************************************************************************************************************/
/* Heure_every_2_hours: Renvoie TRUE une et une seule fois toutes les 2 heures pleines                                        */
/* Entree: ctx - pointeur vers la structure d'état de l'appelant (doit être initialisée à zéro au départ)                     */
/* Sortie: TRUE lors du premier appel correspondant à l'instant heure paire:00:00 ; FALSE le reste du temps                   */
/******************************************************************************************************************************/
 gboolean Heure_every_2_hours ( ABLS_HEURE *ctx )
  { struct tm tm_now;
    time_t now, hour_start;

    if (!ctx) return (FALSE);
    now = time (NULL);
    localtime_r ( &now, &tm_now );

    if (tm_now.tm_min != 0 || tm_now.tm_sec != 0 || (tm_now.tm_hour % 2) != 0) return (FALSE);
    hour_start = now;
    if (ctx->last_triggered == hour_start) return (FALSE);

    ctx->last_triggered = hour_start;
    return (TRUE);
  }
/******************************************************************************************************************************/
/* Heure_every_end_of_day: Renvoie TRUE une et une seule fois à 23:59:59                                                      */
/* Entree: ctx - pointeur vers la structure d'état de l'appelant (doit être initialisée à zéro au départ)                     */
/* Sortie: TRUE lors du premier appel correspondant à l'instant 23:59:59 ; FALSE le reste du temps                            */
/******************************************************************************************************************************/
 gboolean Heure_every_end_of_day ( ABLS_HEURE *ctx )
  { struct tm tm_now;
    time_t now;

    if (!ctx) return (FALSE);
    now = time (NULL);
    localtime_r ( &now, &tm_now );

    if (tm_now.tm_hour != 23 || tm_now.tm_min != 59 || tm_now.tm_sec != 59) return (FALSE);
    if (ctx->last_triggered == now) return (FALSE);

    ctx->last_triggered = now;
    return (TRUE);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
