/******************************************************************************************************************************/
/* src/uuid.c      Fonctions communes autour des UUID — abls-libs                                                            */
/* Projet Abls-Habitat version 1.0       Gestion d'habitat                                                27.07.2026          */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * uuid.c
 * This file is part of Abls-Habitat
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

 #include <uuid/uuid.h>

 #include "uuid.h"

/******************************************************************************************************************************/
/* UUID_New: Genere un nouveau UUID dans le buffer en parametre                                                               */
/* Entree: le buffer a remplir                                                                                                */
/* Sortie: neant                                                                                                              */
/******************************************************************************************************************************/
 void UUID_New ( gchar *target )
  { uuid_t uuid_hex;
    uuid_generate( uuid_hex );
    uuid_unparse_lower( uuid_hex, target );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
