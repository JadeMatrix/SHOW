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

Each of these servers can be tested from a second terminal window.

# `hello_world`

The most basic server possible — returns *200 OK* with a plaintext "Hello World" message for every request.  You can test this server either with `curl`:

```sh
curl -i 0.0.0.0:9090
```

or by navigating to `http://0.0.0.0:9090/` in your browser.

# `echo`

Echoes back the contents of any *POST* request.  Very simple and unsafe — see [`streaming_echo`](#streaming_echo) for a more thorough implementation.  You can test this server with

```sh
curl -i -X POST -d "Knock knock" -H "Content-Type: text/plain" 0.0.0.0:9090
```

# `http_1_1`

Demonstrates how to integrate SHOW's HTTP/1.1 support into your application.  The easiest way to test this server is with [Netcat](https://en.wikipedia.org/wiki/Netcat); after starting the server, connect with

```sh
nc 0.0.0.0 9090
```

Then, before 10 seconds have passed (that's the connection timeout hard-coded into this server), paste at least one HTTP request; you can wait up to 10 seconds between pastes.  Here are two simple requests (note that the two trailing newlines are required for the first one!):

```http
GET / HTTP/1.1
Content-Length: 0


```

```http
POST /foo/bar HTTP/1.1
Content-Length: 11
Content-Type: text/plain

Hello World
```

# `streaming_echo`

A more advanced echo server that streams large requests using `std::istream` and responds using `std::ostream`.  Run & test it like `http_1_1`, but use a longer request you can enter in parts.  For example, first paste this into Netcat:

```http
POST /foo/bar HTTP/1.0
Content-Length: 240
Content-Type: text/plain

AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

```

Then paste this:

```http
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB

```

and finally this:

```http
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

```

The server should echo back each of the three lines as soon as you paste them in.  Note that there is a newline following each of those data chunks.  This might not show up with some Markdown renderers, so try opening this file in a plaintext editor and copying from there.
