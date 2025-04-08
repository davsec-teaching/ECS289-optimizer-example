#!/bin/bash

cd build; make; cd -
clang -c -emit-llvm -S something.c -o something_original.ll
clang -c -emit-llvm -S -fpass-plugin=build/skeleton/SkeletonPass.so something.c
