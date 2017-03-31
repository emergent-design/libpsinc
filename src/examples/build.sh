#!/usr/bin/env bash

for var in "$@"
do
	clang++ -O3 -o ${var%.*} -std=c++14 $var -lpsinc -lfreeimage -lpthread
done
