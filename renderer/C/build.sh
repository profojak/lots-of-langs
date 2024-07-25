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
        exe rm -rf bin/ renderer
        exit 0
    else
        echo -e "\033[1;31merror:\033[0m \033[1minvalid argument:\033[0m $1"
        exit 1
    fi
fi

# Build
FLAGS="-std=c99 -Wall -Wextra -Wpedantic -Werror -O3"
mkdir -p bin/
exe clang $FLAGS -Iinclude -I../../math/C/include src/image.c -c -o bin/image.o
exe clang $FLAGS -Iinclude -I../../math/C/include src/model.c -c -o bin/model.o
exe clang $FLAGS -Iinclude -I../../math/C/include src/shader.c -c -o bin/shader.o
exe clang $FLAGS -Iinclude -I../../math/C/include src/main.c -c -o bin/main.o
exe clang bin/main.o bin/image.o bin/model.o bin/shader.o -lm -o renderer
exit 0
