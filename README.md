# `show::` — Simple Header-Only Webserver

[![stable version](https://img.shields.io/github/release/JadeMatrix/SHOW.svg?label=stable)](https://github.com/JadeMatrix/SHOW/releases/latest)
[![latest version](https://img.shields.io/github/release/JadeMatrix/SHOW/all.svg?label=latest)](https://github.com/JadeMatrix/SHOW/releases)

The goal of SHOW is to make an idiomatic library for standalone webserver applications written for modern C++.  SHOW is simple in the same way the standard library is simple — it doesn't make any design decisions for the programmer, instead giving them whatever tools they need to build their application.

Both HTTP/1.0 and HTTP/1.1 are supported.  SHOW assumes a modern approach to application hosting, and is intended to be run behind a full reverse proxy such as [nginx](https://nginx.org/).  As such, SHOW will not support HTTP/2 or TLS (HTTPS).  Instead, you should write your applications to serve local HTTP/1.x requests.

SHOW uses the [`zlib` license](LICENSE).  C++11 support and a POSIX operating system are required.

## How to use

You can find SHOW's documentation, including a tutorial, on [ReadTheDocs.io](http://show-cpp.readthedocs.io/).

SHOW is designed to give the programmer full control over how and where HTTP requests are handled.  Any number of servers can be created which will then accept connections.  From each connection, one or more requests can be extracted, each of which can then be responded to separately.  Everything — when to serve, what requests to accept, even whether to send a response at all — is completely up to the application.

There are a few important points to keep in mind, however:

HTTP/1.1 — and HTTP/1.0 with an extension — allow multiple requests to be pipelined on the same TCP connection.  Don't try to extract another request from a connection before you've finished handling the last one (the `response` object has been destroyed)!  If you don't want to even send a response for a given pipelined request, you must close the connection, otherwise the client will think you're responding to the first unanswered request.  SHOW can't know with certainty where on the connection one request ends and another starts — it's just the nature of pipelined HTTP.  Sure, the *Content-Length* header could be used, and [chunked transfer encoding](https://en.wikipedia.org/wiki/Chunked_transfer_encoding) has well-established semantics, but if neither are used it is up to your application to figure out the end of the request's content.  In general, you should reject requests whose length you can't readily figure out, but SHOW leaves that decision up to the programmer.

Another thing SHOW doesn't prevent you from doing is creating multiple responses to the same request.  This is mainly for simplicity and because it's very unlikely to happen in a well-structured program.  Doing this may throw an exception in the future; for now, just don't do it!
