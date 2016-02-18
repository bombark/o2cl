#!/bin/bash 

cd "$nom"

codename=$(tiobj "@.sysobj.ti" get "codename")

reprepro list $codename $_Akk
