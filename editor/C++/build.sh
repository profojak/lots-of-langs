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
exe clang++ $FLAGS -x c++-module include/Input.ccm --precompile -o bin/Input.pcm
exe clang++ $FLAGS -x c++-module include/Terminal.ccm --precompile -fprebuilt-module-path=bin/ -o bin/Terminal.pcm
exe clang++ $FLAGS -x c++-module include/Output.ccm --precompile -fprebuilt-module-path=bin/ -o bin/Output.pcm
exe clang++ $FLAGS src/Terminal.cc -fprebuilt-module-path=bin/ -c -o bin/Terminal-src.o
exe clang++ $FLAGS src/Input.cc -fprebuilt-module-path=bin/ -c -o bin/Input-src.o
exe clang++ $FLAGS src/Output.cc -fprebuilt-module-path=bin/ -c -o bin/Output-src.o
exe clang++ $FLAGS src/main.cc -fprebuilt-module-path=bin/ -c -o bin/main.o
exe clang++ $FLAGS bin/Terminal.pcm -fprebuilt-module-path=bin/ -c -o bin/Terminal.o
exe clang++ $FLAGS bin/Input.pcm -fprebuilt-module-path=bin/ -c -o bin/Input.o
exe clang++ $FLAGS bin/Output.pcm -fprebuilt-module-path=bin/ -c -o bin/Output.o
exe clang++ bin/main.o bin/Terminal.o bin/Terminal-src.o bin/Input.o bin/Input-src.o bin/Output.o bin/Output-src.o -o editor
exit 0
