# `show::` — Simple Header-Only Webserver

[![stable version](https://img.shields.io/github/release/JadeMatrix/SHOW.svg?label=stable)](https://github.com/JadeMatrix/SHOW/releases/latest)
[![latest version](https://img.shields.io/github/release/JadeMatrix/SHOW/all.svg?label=latest)](https://github.com/JadeMatrix/SHOW/releases)

The goal of SHOW is to make an idiomatic library for standalone webserver applications written for modern C++.  SHOW is simple in the same way the standard library is simple — it doesn't make any design decisions for the programmer, instead giving them whatever tools they need to build their application.

Both HTTP/1.0 and HTTP/1.1 are supported.  SHOW assumes a modern approach to application hosting, and is intended to be run behind a full reverse proxy such as [nginx](https://nginx.org/).  As such, SHOW will not support HTTP/2 or TLS (HTTPS).  Instead, you should write your applications to serve local HTTP/1.x requests.

SHOW uses the [`zlib` license](LICENSE).  C++11 support and a POSIX operating system are required.

## How to use

*This shows the basic usage of SHOW; see the [examples](examples) for more advanced stuff*

SHOW is designed to give the programmer full control over how and where HTTP requests are handled.  Any number of servers can be created which will then accept connections.  From each connection, one or more requests can be extracted, each of which can then be responded to separately.  Everything — when to serve, what requests to accept, even whether to send a response at all — is completely up to the application.

There are a few important points to keep in mind, however:

HTTP/1.1 — and HTTP/1.0 with an extension — allow multiple requests to be pipelined on the same TCP connection.  Don't try to extract another request from a connection before you've finished handling the last one (the `response` object has been destroyed)!  If you don't want to even send a response for a given pipelined request, you must close the connection, otherwise the client will think you're responding to the first unanswered request.  SHOW can't know with certainty where on the connection one request ends and another starts — it's just the nature of pipelined HTTP.  Sure, the *Content-Length* header could be used, and [chunked transfer encoding](https://en.wikipedia.org/wiki/Chunked_transfer_encoding) has well-established semantics, but if neither are used it is up to your application to figure out the end of the request's content.  In general, you should reject requests whose length you can't readily figure out, but SHOW leaves that decision up to the programmer.

Another thing SHOW doesn't prevent you from doing is creating multiple responses to the same request.  This is mainly for simplicity and because it's very unlikely to happen in a well-structured program.  Doing this may throw an exception in the future; for now, just don't do it!

### Including

SHOW is entirely contained in a single file, so all you have to do is include it in your compiler's search path and add

```cpp
#include <show.hpp>
```

in your source files as needed.

Adding header search paths will depend on the compiler, but for GCC and Clang simply add an `-I` option:

```sh
clang++ -I "/directory/containing/SHOW/header" my_server.cpp -o my_server
```

### Servers

To start serving requests, first create a server object:

```cpp
show::server my_server(
    "0.0.0.0",  // IP address on which to serve
    9090,       // Port on which to serve
    10          // Listen timeout, in seconds
);
```

That's it, you've made a server that sits there and hangs on the first connection it gets (as long as it arrives within 10 seconds).  Not terribly useful, but that's easy to fix.

### Connections

For each call of `my_server.serve()` a single `show::connection` object will be returned or a `show::connection_timeout` thrown.  You may want to use something like this:

```cpp
while( true )
    try
    {
        show::connection connection( my_server.serve() );
        // handle request(s) here
    }
    catch( show::connection_timeout& ct )
    {
        std::cout
            << "timed out waiting for a connection, looping..."
            << std::endl
        ;
        continue;
    }
```

The server listen timeout can be a positive number, `0`, or `-1`.  If it is `-1`, the server will continue listening until interrupted by a signal; if `0`, `serve()` will throw a `show::connection_timeout` immediately unless connections are available.

The connection is now independent from the server.  You can adjust the connection's timeout independently using `connection::timeout(int)`.  You can also pass it off to a worker thread for processing so your server can continue accepting other connections; this is usually how you'd implement a real web application.

### Requests

`show::request` objects have a number of `const` fields containing the HTTP request's metadata:

| Field | Description |
| --- | --- |
| `protocol` | An enum value, either `HTTP_1_0`, `HTTP_1_1`, `NONE`, or `UNKNOWN` |
| `protocol_string` | The raw protocol string, useful if `protocol` is `UNKNOWN` |
| `method` | An uppercase `std::string` containing the [request method](https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol#Request_methods) (`GET`, `POST`, etc.) |
| `path` | The request path split into a `std::vector` of `std::string`s, with [URL-decoding](https://en.wikipedia.org/wiki/Percent-encoding) already handled |
| `query_args` | The request query arguments split into a `std::map` of `std::vector`s of `std::string`s, with URL-decoding already handled |
| `headers` | The request headers split into a `std::map` of `std::vector`s of `std::string`s |
| `unknown_content_length` | An enum value, either `NO` (length is given), `YES` (length is missing), or `MAYBE` (length is indeterminate or corrupt), with `YES` and `MAYBE` evaluating to `true` |
| `content_length` | The raw content length string, useful if `unknown_content_length` is `MAYBE` |

`query_args` and `headers` are somewhat complicated, as each key can appear multiple times and so have multiple values.  [`std::map< std::string, std::vector< std::string > >`](http://en.cppreference.com/w/cpp/container/map) was chosen over than [`std::multimap< std::string, std::string >`](http://en.cppreference.com/w/cpp/container/multimap) as the former is easier for read operations (`request`s are generally read-only).

Note that the above fields do not include the request content, if any.  This is because HTTP allows the request content to be streamed to the server.  In other words, the server can interpret the headers then wait for the client to send data over a period of time.  For this purpose, `show::request` inherits from [`std::streambuf`](http://en.cppreference.com/w/cpp/io/basic_streambuf), implementing the read/get functionality.  You can use the raw `std::streambuf` methods to read the incoming data, or create a [`std::istream`](http://en.cppreference.com/w/cpp/io/basic_istream) from the request object for `std::cin`-like behavior.

For example, if your server is expecting the client to `POST` a single integer, you can use:

```cpp
show::request request( test_server.serve() );

std::istream request_content_stream( request );

int my_integer;
request_content_stream >> my_integer;
```

Please note that the above is not terribly safe; production code should include various checks to guard against buggy or malignant clients.

Also note that individual request operations may timeout, so the entire serve code should look like this:

```cpp
while( true )
    try
    {
        show::connection connection( my_server.serve() );
        show::request request( connection );
        std::istream request_content_stream( request );
        try
        {
            int my_integer;
            request_content_stream >> my_integer;
            std::cout << "client sent " << my_integer << "\n";
        }
        catch( show::connection_timeout& ct )
        {
            std::cout << "got a request, but client timed out!\n";
        }
    }
    catch( show::connection_timeout& ct )
    {
        std::cout
            << "timed out waiting for a connection, looping..."
            << std::endl
        ;
        continue;
    }
```

### Responses

Sending responses is slightly more complex than reading basic requests.  Say you want to send a "Hello World" message for any incoming request.  First, start with a string containing the response message:

```cpp
std::string response_content = "Hello World";
```

Next, create a headers object to hold the content type and length headers (note that header values must be strings):

```cpp
show::headers_t headers = {
    { "Content-Type", { "text/plain" } },
    { "Content-Length", {
        std::to_string( response_content.size() )
    } }
};
```

Since it's a `std::map`, you can also add headers to a `show::headers_t` like this:

```cpp
headers[ "Content-Type" ].push_back( "text/plain" );
```

Then, set the [HTTP status code](https://en.wikipedia.org/wiki/List_of_HTTP_status_codes) for the response to the generic *200 OK*:

```cpp
show::response_code code = {
    200,
    "OK"
};
```

Creating a response object requires the headers and response code to have been decided already, as they are marshalled and buffered for sending as soon as the object is created.  A response object also needs to know which request it is in response to.  While there's nothing preventing you from creating multiple responses to a single request this way, that will most certainly break your server.

```cpp
show::response response(
    request,
    show::http_protocol::HTTP_1_0,
    code,
    headers
);
```

Finally, send the response content.  Here, a [`std::ostream`](http://en.cppreference.com/w/cpp/io/basic_ostream) is used, as `show::response` inherits from and implements the write/put functionality of `std::streambuf`:

```cpp
std::ostream response_stream( &response );
response_stream << response_content;
```

### Version

Version information can be found in the `show::version` struct:

| Field | Description |
| --- | --- |
| `show::version.name` | Name of the server — `"SHOW"` — as a `std::string` |
| `show::version.major` | The major version as an `int` |
| `show::version.minor` | The minor version as an `int` |
| `show::version.revision` | The revision as an `int` |
| `show::version.string` | `"major.minor.revision"` as a `std::string` |
