#!/bin/bash

if [ "$nom" == "." ]; then
	akk_url=$(realpath "$_Akk")
	cd $(remote default)
else
	akk_url=$_Akk
	cd "$nom"
fi

url=$(tiobj "@.sysobj.ti" get url)
user=$(tiobj "@.sysobj.ti" get user)

if [ "$url" == "" ]; then
	echo "You must set url server in the file .sysobj.ti";
	exit 1;
fi




scp "$akk_url" "$user@$url:"
