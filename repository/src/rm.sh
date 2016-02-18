#!/bin/bash


if [[ "$nom" == "." ]]; then
	cd $(repository default)
else
	cd $nom
fi

codename=$(tiobj "@.sysobj.ti" get "codename")
reprepro remove $codename $_Akk
