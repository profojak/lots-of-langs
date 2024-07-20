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
FLAGS="-std=c99 -Wall -Wextra -Wpedantic -Werror -O3"
mkdir -p bin/
exe clang $FLAGS -Iinclude src/input.c -c -o bin/input.o
exe clang $FLAGS -Iinclude src/output.c -c -o bin/output.o
exe clang $FLAGS -Iinclude src/terminal.c -c -o bin/terminal.o
exe clang $FLAGS -Iinclude src/main.c -c -o bin/main.o
exe clang bin/main.o bin/input.o bin/output.o bin/terminal.o -o editor
exit 0
