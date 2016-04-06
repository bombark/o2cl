#!/bin/bash

if [[ "$nom" == "." ]]; then
	deb_url=$(realpath $_Akk)
	cd $(repository default)
else
	deb_url=$_Akk
	cd $nom
fi

codename=$(tiobj "@.sysobj.ti" get codename);
reprepro includedeb $codename "$deb_url"
