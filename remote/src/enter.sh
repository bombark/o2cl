#!/bin/bash


if [ "$nom" == "." ]; then
	cd $(remote default)
else
	cd "$nom"
fi



url=$(tiobj "@.sysobj.ti" get url)
user=$(tiobj "@.sysobj.ti" get user)

if [ "$url" == "" ]; then
	echo "You must set url server in the file .sysobj.ti";
	exit 1;
fi


ssh "$user@$url"
