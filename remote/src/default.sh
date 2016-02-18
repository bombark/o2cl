#!/bin/bash

if [ -e "$HOME/.o2cl/remote/default" ]; then
	echo $(cat "$HOME/.o2cl/remote/default")
else
	exit 1
fi
