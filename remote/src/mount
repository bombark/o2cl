#!/bin/bash

cd "$nom"
url=$(tiobj "@.sysobj.ti" get url)
user=$(tiobj "@.sysobj.ti" get user)

if [ "$url" == "" ]; then
	echo "You must set url server in the file .sysobj.ti";
	exit 1;
fi


mkdir -p "data"
sshfs "$user@$url:" "data"
