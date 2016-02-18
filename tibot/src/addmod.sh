#!/bin/bash

cd $nom

name=$_Akk;
if [ ! -e "$class_url/etc/$name" ]; then
	echo "Module $name don't exist"
	exit 1;
fi

cp "$class_url/etc/$name/bin/$name" "./bin/" 
