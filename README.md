# ABLS-LIBS

Bibliotheque C partagee pour Abls-Habitat.

Le projet contient trois modules metier principaux:

- `info`: journalisation structuree vers syslog
- `json`: helpers de manipulation JSON (json-glib)
- `mqtt`: couche MQTT partagee (libmosquitto)

## Etat du build

Le build CMake actuel compile et installe la librairie avec `info`, `json` et `mqtt`.

- Sources compilees: `src/info.c`, `src/json.c`, `src/mqtt.c`
- Headers installes: `include/info.h`, `include/json.h`, `include/mqtt.h`, `include/abls-libs.h`
- Nom pkg-config: `abls-libs`

## Installation rapide

```sh
./install_deps.sh
./build.sh
sudo ./install.sh
```

## Utilisation via pkg-config

```sh
pkg-config --cflags --libs abls-libs
```

Exemple de compilation:

```sh
cc app.c -o app $(pkg-config --cflags --libs abls-libs)
```

## Module Info

Header: `include/info.h`

Objectif: emission de logs JSON vers syslog, avec filtrage par niveau et forcage debug par `facility`.

### API publique

```c
void Info_init(const gchar *entete, const gchar *prefixe_name, guint log_level);
void Info_change_log_level(guint new_log_level);

void Info(const gchar *function, const gchar *facility, const gchar *prefixe,
                    guint priority, const gchar *format, ...);

gint Info_reset_nbr_log(void);

void Info_debug_facility(const gchar *prefixe, const gchar *context);
void Info_undebug_facility(const gchar *prefixe, const gchar *context);
void Info_clear_debug_facilities(void);
```

### Comportement

- `Info_init` initialise syslog, configure le niveau global et enregistre un cleanup `on_exit`.
- `Info` emet un JSON contenant `thread`, puis optionnellement `facility`, `function`, et la cle de prefixe definie par `prefixe_name`.
- Le message est logge si:
    - son `priority` est <= au niveau courant, ou
    - la `facility` est forcee en debug.
- `Info_reset_nbr_log` retourne le nombre de logs emis depuis le dernier reset et remet le compteur a zero.
- Les facilities forcees debug sont gerees de maniere thread-safe.

### Exemple

```c
#include <syslog.h>
#include <abls-libs/info.h>

Info_init("my-service", "thread_tech_id", LOG_INFO);
Info(__func__, "json", "main", LOG_NOTICE, "Service started");
```

### Format de log

Exemple de payload envoye a syslog:

```json
{
    "thread": "worker-1",
    "thread_tech_id": "main",
    "facility": "json",
    "function": "LoadConfig",
    "message": "Config loaded"
}
```

## Module Json

Header: `include/json.h`

Objectif: faciliter la creation, lecture, extraction et persistance de `JsonNode`.

### API publique (principales fonctions)

Creation et cycle de vie:

```c
JsonNode *Json_create(void);
void Json_unref(JsonNode *RootNode);
JsonNode *Json_get_from_string(const gchar *chaine);
gchar *Json_to_string(JsonNode *RootNode);
```

Ajout de membres:

```c
void Json_add_string(JsonNode *RootNode, gchar *name, const gchar *chaine);
void Json_add_bool(JsonNode *RootNode, gchar *name, gboolean valeur);
void Json_add_double(JsonNode *RootNode, gchar *name, gdouble valeur);
void Json_add_int(JsonNode *RootNode, gchar *name, gint64 valeur);
void Json_add_null(JsonNode *RootNode, gchar *name);
JsonArray *Json_add_array(JsonNode *RootNode, gchar *name);
JsonNode *Json_add_object(JsonNode *RootNode, gchar *name);
```

Accesseurs:

```c
gchar *Json_get_string(JsonNode *RootNode, gchar *chaine);
gdouble Json_get_double(JsonNode *RootNode, gchar *chaine);
gboolean Json_get_bool(JsonNode *RootNode, gchar *chaine);
gint Json_get_int(JsonNode *RootNode, gchar *chaine);
JsonArray *Json_get_array(JsonNode *RootNode, gchar *chaine);
JsonObject *Json_get_object_as_object(JsonNode *RootNode, gchar *chaine);
JsonNode *Json_get_object_as_node(JsonNode *RootNode, gchar *chaine);
gboolean Json_has_member(JsonNode *RootNode, gchar *chaine);
```

Tableaux:

```c
void Json_array_add_element(JsonArray *array, JsonNode *element);
void Json_array_add_one_element(JsonNode *RootNode, gchar *array_name, JsonNode *element);
void Json_array_del_one_element(JsonNode *RootNode, gchar *array_name, guint index);
JsonNode *Json_array_get_element_at(JsonNode *RootNode, gchar *array_name, guint index);
guint Json_array_get_length(JsonNode *RootNode, gchar *array_name);
void Json_foreach_array_element(JsonNode *RootNode, gchar *array_name,
                                                                JsonArrayForeach fonction, gpointer data);
```

Fichiers:

```c
JsonNode *Json_read_from_file(gchar *filename);
gboolean Json_write_to_file(gchar *filename, JsonNode *RootNode);
void Json_read_config(gchar *filename, JsonNode *target);
```

### Regles de memoire

- `Json_create` et `Json_get_from_string` retournent un node a liberer avec `Json_unref`.
- `Json_to_string` retourne une chaine allouee a liberer avec `g_free`.
- Les accesseurs `Json_get_*` retournent des valeurs liees au `JsonNode` source (ne pas liberer les pointeurs retournes).

### Exemple

```c
#include <abls-libs/json.h>

JsonNode *msg = Json_create();
Json_add_string(msg, "service", "api");
Json_add_int(msg, "status", 200);

gchar *payload = Json_to_string(msg);
g_free(payload);
Json_unref(msg);
```

## Module Mqtt

Header: `include/mqtt.h`

Objectif: encapsuler la session mosquitto, les abonnements, une file asynchrone de messages JSON et la logique de reconnexion.

### API publique

```c
struct ABLS_MQTT *Mqtt_init(const gchar *log_facility, const gchar *log_prefixe,
                                                        const gchar *client_id, gboolean is_ssl,
                                                        const gchar *ca_file, const gchar *ca_path,
                                                        const gchar *username, const gchar *password,
                                                        const gchar *hostname, gint port, gint qos);

gboolean Mqtt_start(struct ABLS_MQTT *mqtt);
void Mqtt_stop(struct ABLS_MQTT *mqtt);

void Mqtt_subscribe(struct ABLS_MQTT *mqtt, gchar *format, ...);
void Mqtt_unsubscribe(struct ABLS_MQTT *mqtt, gchar *format, ...);

void Mqtt_send_message(struct ABLS_MQTT *mqtt, JsonNode *node,
                                             gboolean retain, gchar *topic, ...);
JsonNode *Mqtt_get_message(struct ABLS_MQTT *mqtt);
```

### Comportement attendu

- `Mqtt_init` prepare la session et configure les callbacks mosquitto.
- `Mqtt_start` connecte au broker puis demarre la boucle asynchrone.
- `Mqtt_subscribe` et `Mqtt_unsubscribe` manipulent une liste de topics protegee par lock.
- `Mqtt_get_message` lit la file de reception sans blocage.
- `Mqtt_send_message` publie un payload JSON et peut conserver en file les messages en echec de publication.

### Notes importantes

- Le code MQTT present dans `src/mqtt.c` contient des incoherences de nommage et d'integration qui doivent etre stabilisees avant activation dans la cible CMake.
- Tant que `mqtt` n'est pas ajoute a la cible partagee, il ne fait pas partie de la librairie distribuee via `abls-libs`.

## Header parapluie

`include/abls-libs.h` inclut actuellement:

- `info.h`
- `json.h`

Le header `mqtt.h` doit etre inclus explicitement si ce module est utilise hors cible courante.

## Packaging RPM

```sh
cd build && cpack -G RPM
```

Produit:

- `abls-libs-X.Y.Z-1.rpm` (runtime)
- `abls-libs-devel-X.Y.Z-1.rpm` (headers + pkg-config)

## Packaging DEB (Debian/Raspberry)

Install dependencies:

```sh
sudo ./install_deps.sh
```

Build DEB packages:

```sh
./build_apt.sh --dist bookworm
```

Generated packages:

- `abls-libs_VERSION_ARCH.deb` (runtime)
- `abls-libs-dev_VERSION_ARCH.deb` (headers + pkg-config)

Artifacts are copied to:

- `build/deb/<suite>/<arch>/`

## Install depuis GitHub Release

```sh
dnf install https://github.com/sebaru/abls-libs/releases/download/v1.0-0/abls-libs-devel-1.0.0-1.rpm
```
