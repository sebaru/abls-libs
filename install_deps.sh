#!/bin/sh

SOCLE=`grep "^ID=" /etc/os-release | cut -f 2 -d '='`

if [ "$(whoami)" != "root" ]
 then
	 echo "Only user root can run this script (or sudo)."
	 exit 1
fi

groupadd abls

if [ "$SOCLE" = "fedora" ]
 then
	echo "Installing Fedora dependencies"
	dnf install -y glib2-devel cmake rpm-build git
fi
