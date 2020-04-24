#!/bin/bash

clang++ -O3 -o psinc-lb -std=c++17 psinc-loopback.cpp -lpsinc -lfreeimage -lpthread -lncurses -limp
# -lavformat -lavcodec -lavutil
# -lswscale
