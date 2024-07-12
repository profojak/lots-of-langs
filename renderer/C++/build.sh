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
        exe rm -rf bin/ renderer
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
exe clang++ $FLAGS -x c++-module include/Image.ccm --precompile $MODULES -o bin/Image.pcm
exe clang++ $FLAGS src/Image.cc $MODULES -c -o bin/Image-src.o
exe clang++ $FLAGS src/main.cc $MODULES -c -o bin/main.o
exe clang++ $FLAGS bin/Image.pcm $MODULES -c -o bin/Image.o
exe clang++ bin/main.o bin/Image.o bin/Image-src.o -o renderer
exit 0
