#!/bin/bash

mkdir "$_Akk"
cd "$_Akk"
echo "class=proj:app"  > ".sysobj.ti"
echo "name='$_Akk'"   >> ".sysobj.ti"
echo "App{name='$_Akk';src='src/main.cpp'}" >> ".sysobj.ti"

mkdir "src"
cp "$class_url/etc/main.cpp" "src/main.cpp"


