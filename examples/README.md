There are two easy ways to build the examples in this directory.  If you have [`cmake`](https://cmake.org/) installed, there are build instructions in the supplied *CMakeLists.txt*.  To use it, first run 

```sh
mkdir -p make
cd make
cmake ..
```

then either run `make` to build all examples, or `make $NAME` to build a specific example.  The other way to build any of the examples is manually with Clang (`clang++`) or GCC (`g++`).  Assuming you're running this in the "make" directory from above:

```sh
clang++ -std=c++11 -I ../src ../examples/$NAME.cpp -o $NAME
```

# `hello_world`

The most basic server possible — returns *200 OK* with a plaintext "Hello World" message for every request.

# `echo`

Echoes back the contents of any *POST* request.  Very simple and unsafe — see [`streaming_echo`](#streaming_echo) for a more thorough implementation.