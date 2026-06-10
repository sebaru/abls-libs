# abls-habitat-libs

Shared C library for the Abls-Habitat project, providing a unified structured logging system used by `abls-habitat-api` and `Watchdogd`.

## Features

- Structured JSON logging to syslog
- Per-context debug forcing via `Info_force_debug()`
- Thread-safe (GLib atomics + GMutex)
- Log message rate counter (`Info_reset_nbr_log`)
- pkg-config integration

## API

```c
void Info_init           (const gchar *ident, guint log_level);
void Info_new            (const gchar *function, const gchar *context, guint priority,
                          const gchar *domain_uuid, const gchar *format, ...);
void Info_change_log_level(guint new_log_level);
void Info_stop           (void);
gint Info_reset_nbr_log  (void);
void Info_force_debug    (const gchar *context);
void Info_unforce_debug  (const gchar *context);
void Info_clear_forced_debug(void);
```

## JSON log format

```json
{ "context": "smsg", "thread": "Watchdogd", "function": "Run_thread", "message": "SMS System is running" }
```

## Build

```sh
./install_deps.sh
./build.sh
sudo ./install.sh
```

## RPM packaging

```sh
cd build && cpack -G RPM
```

Produces `abls-habitat-libs-X.Y.Z-1.rpm` (runtime) and `abls-habitat-libs-devel-X.Y.Z-1.rpm` (headers + pkg-config).

## Install from GitHub Release

```sh
dnf install https://github.com/sebaru/abls-habitat-libs/releases/download/v1.0-0/abls-habitat-libs-devel-1.0.0-1.rpm
```
