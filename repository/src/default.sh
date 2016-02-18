#!/bin/bash

if [ ! -e "$HOME/.o2cl/repository/default" ]; then
	exit 1
fi

cat "$HOME/.o2cl/repository/default"
