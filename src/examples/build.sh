#!/usr/bin/env bash

for var in "$@"; do
	if [ "$var" = "control-flash.cpp" ]; then
		clang++ -O3 -o ${var%.*} -std=c++14 $var -lserialport -lpthread
	else
		clang++ -O3 -o ${var%.*} -std=c++14 $var -lpsinc -lfreeimage -lpthread
	fi
done

