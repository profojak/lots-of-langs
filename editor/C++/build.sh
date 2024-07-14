#!/bin/bash

# Print and execute command and exit on fail
exe() {
    echo "$@";
    "$@";
    if [ "$?" -ne 0 ]; then
        exit 1
    fi
}

# Clean
if [ -n "$1" ]; then
    if [[ "$1" == "clean" ]]; then
        exe rm -rf bin/ editor
        exit 0
    else
        echo -e "\033[1;31merror:\033[0m \033[1minvalid argument:\033[0m $1"
        exit 1
    fi
fi

# Build
FLAGS="-std=c++23 -Wall -Wextra -Wpedantic -Werror -O3"
mkdir -p bin/
exe clang++ $FLAGS -x c++-module include/Terminal.ccm --precompile -o bin/Terminal.pcm
exe clang++ $FLAGS src/Terminal.cc -fprebuilt-module-path=bin/ -c -o bin/Terminal-src.o
exe clang++ $FLAGS src/main.cc -fprebuilt-module-path=bin/ -c -o bin/main.o
exe clang++ $FLAGS bin/Terminal.pcm -fprebuilt-module-path=bin/ -c -o bin/Terminal.o
exe clang++ bin/main.o bin/Terminal.o bin/Terminal-src.o -o editor
exit 0
