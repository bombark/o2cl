#!/bin/bash

mkdir "$_Akk"
cd "$_Akk"
echo "class=proj:ticlass"  > ".sysobj.ti"
echo "name='$_Akk'"       >> ".sysobj.ti"

mkdir "src"
touch "src/Makefile"

