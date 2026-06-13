# abls-libs

Shared C library for the Abls-Habitat project, providing a unified structured logging system used by `abls-habitat-api` and `Watchdogd`.

## Features

- Structured JSON logging to syslog
- Per-context debug forcing via `abls_info_debug_context()`
- Thread-safe (GLib atomics + GMutex)
- Log message rate counter (`abls_info_reset_nbr_log`)
- pkg-config integration

## API

```c
void abls_info_init             (const gchar *ident, const gchar *perimeter_name, guint log_level);
void abls_info_new              (const gchar *function, const gchar *context, guint priority,
                                 const gchar *format, ...);
void abls_info_change_log_level (guint new_log_level);
gint abls_info_reset_nbr_log    (void);
void abls_info_debug_context    (const gchar *context);
void abls_info_undebug_context  (const gchar *context);
void abls_info_clear_debug_contexts (void);
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

Produces `abls-libs-X.Y.Z-1.rpm` (runtime) and `abls-libs-devel-X.Y.Z-1.rpm` (headers + pkg-config).

## Install from GitHub Release

```sh
dnf install https://github.com/sebaru/abls-libs/releases/download/v1.0-0/abls-libs-devel-1.0.0-1.rpm
```
