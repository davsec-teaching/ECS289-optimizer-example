# llvm-pass-skeleton

Replaces less frequenstly used heap objects with a call to `slow_malloc` in the LLVM IR.

Build:

    $ cd llvm-pass-skeleton
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ cd ..

Run:

    $ ./run.sh
