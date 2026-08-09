/******************************************************************************************************************************/
/* include/run.h       Declaration des helpers d'execution fork/exec — abls-libs                                            */
/* Projet Abls-Habitat version 1.0       Gestion d'habitat                                                22.07.2026          */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * run.h
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

#ifndef _ABLS_RUN_H_
 #define _ABLS_RUN_H_

 #include <glib.h>

 #define FACILITY_RUN  "run"

/*-- Execution de processus --------------------------------------------------------------------------------------------------*/
/* Les arguments doivent etre termines par NULL. Retourne le code de sortie du programme lance, -1 si erreur interne.       */
 extern gint     Run_shell       ( const gchar *command, ... );

/*-- Execution de threads ----------------------------------------------------------------------------------------------------*/
 extern gpointer Run_thread_join ( const gchar *name, GThreadFunc func, gpointer data );

#endif /* _ABLS_RUN_H_ */
/*----------------------------------------------------------------------------------------------------------------------------*/