#!/bin/bash

mkdir "$_Akk"
cd "$_Akk"
echo "class=proj:lib" > ".sysobj.ti"
echo "name='$_Akk'"  >> ".sysobj.ti"

mkdir "src"
mkdir "include"
mkdir "etc"
cp "$class_url/etc/lib-postinst.sh" "etc/postinst"

