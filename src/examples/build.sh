#!/usr/bin/env bash

for var in "$@"; do
	if [ "$var" = "control-flash.cpp" ]; then
		clang++ -g -O3 -o ${var%.*} -std=c++17 $var -lserialport -lpthread
	else
		clang++ -g -O3 -o ${var%.*} -std=c++17 $var -lpsinc -lfreeimage -lpthread
	fi
done

