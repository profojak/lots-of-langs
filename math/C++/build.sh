#!/bin/bash

# Print and execute command
exe() { echo "$@"; "$@"; }

# Clean
if [ -n "$1" ]; then
    if [[ "$1" == "clean" ]]; then
        exe rm -rf bin/ libmath.a
        exit 0
    else
        echo -e "\033[1;31merror:\033[0m \033[1minvalid argument:\033[0m $1"
        exit 1
    fi
fi

# Build
FLAGS="-std=c++23 -Wall -Wextra -Wpedantic -Werror -O3"
mkdir -p bin/
exe clang++ $FLAGS -x c++-module src/Vector.cc --precompile -o bin/Vector.pcm
exe clang++ $FLAGS bin/Vector.pcm -fprebuilt-module-path=bin/ -c -o bin/Vector.o
exe llvm-ar rcs libmath.a bin/Vector.o
exit 0
