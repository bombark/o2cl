#!/bin/bash

cd $nom
tiobj "@.sysobj.ti" replace "$class_url/etc/distributions" > "./conf/distributions"
