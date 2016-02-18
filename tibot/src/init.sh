#!/bin/bash

cd "$nom"
mkdir -p "/run/shm/tibot-objs"
ln -s "/run/shm/tibot-objs" "objs"
