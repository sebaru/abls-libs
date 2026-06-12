#!/bin/bash
# install_deps.sh — Installe les dépendances de build pour abls-libs (Fedora/RHEL)
set -e
dnf install -y glib2-devel cmake rpm-build git
