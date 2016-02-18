#!/bin/bash

cd $nom

private_key=$(gpg --list-keys "$_Akk" | grep "sub" | tr -s ' ' | cut -d' ' -f2 | cut -d'/' -f2)

public_key=$(gpg --list-keys "$_Akk" | grep "pub" | tr -s ' ' | cut -d' ' -f2 | cut -d'/' -f2)

gpg --armor --output "./conf/gpg.key" --export "$public_key" 

tiobj "@.sysobj.ti" set "key='$private_key'" to ".sysobj.ti"
