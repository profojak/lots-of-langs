#!/bin/bash

LIB="../../math/C++"

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
        exe rm -rf bin/ raytracer
        (cd $LIB && exe ./build.sh clean)
        exit 0
    else
        echo -e "\033[1;31merror:\033[0m \033[1minvalid argument:\033[0m $1"
        exit 1
    fi
fi

# Build
FLAGS="-std=c++23 -Wall -Wextra -Wpedantic -Werror -O3"
MODULES="-fprebuilt-module-path=bin -fprebuilt-module-path=$LIB/bin"
mkdir -p bin/
(cd $LIB && exe ./build.sh)
exe clang++ $FLAGS -x c++-module include/Camera.ccm --precompile $MODULES -o bin/Camera.pcm
exe clang++ $FLAGS src/Camera.cc $MODULES -c -o bin/Camera-src.o
exe clang++ $FLAGS src/main.cc $MODULES -c -o bin/main.o
exe clang++ $FLAGS bin/Camera.pcm $MODULES -c -o bin/Camera.o
exe clang++ bin/main.o bin/Camera.o bin/Camera-src.o -o raytracer
exit 0
