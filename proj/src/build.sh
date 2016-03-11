#!/bin/bash

cd "$nom"

if [ ! -e "build" ]; then
	proj make
fi

sysobj_to=$(tiobj "@.sysobj.ti" get "prefix_install")

echo $sysobj_to

cd "build";
make install;



if [ -e ".tmp" ]; then
	rm -rf ".tmp"
fi
mkdir -p ".tmp"



if [ "$to" != "" ]; then
	to=".tmp/$to";
elif [ "$sysobj_to" != "" ]; then
	to=".tmp/$sysobj_to";
else
	to=".tmp/"
fi
mkdir -p "$to"

rsync -r ".builded/" "$to"



mkdir -p ".tmp/DEBIAN"
proj gen/control > ".tmp/DEBIAN/control"

if [ -e "../etc/postinst.sh" ]; then
	cp "../etc/postinst.sh" ".tmp/DEBIAN/postinst"
	chmod +x ".tmp/DEBIAN/postinst"
fi

if [ -e "../etc/postrm.sh" ]; then
	cp "../etc/postrm.sh" ".tmp/DEBIAN/postrm"
	chmod +x ".tmp/DEBIAN/postrm"
fi



name=$(tiobj "@../.sysobj.ti" get name | tr '[:upper:]' '[:lower:]')
dpkg-deb -b ".tmp"
mv ".tmp.deb" "../$name-amd64.deb"

