#!/bin/bash


cd "$nom"
if [[ $(node is "ticlass") ]]; then
	mkdir -p "build"
	proj gen/make > "build/Makefile" 
	cd "build"
	make;

else
	if [ ! -e "CMakeLists.txt" ]; then
		proj gen/cmake > "CMakeLists.txt"
	fi

	if [ ! -e "build" ]; then
		mkdir  build
		mkdir "$nom/build/.builded"
		cd "build"
		cmake .. -DCMAKE_INSTALL_PREFIX="$nom/build/.builded"
		cd - > "/dev/null"
	fi

	cd "build"
	make

fi
