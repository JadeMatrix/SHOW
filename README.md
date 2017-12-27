# `show::` — Simple Header-Only Webserver

[![stable version](https://img.shields.io/github/release/JadeMatrix/SHOW.svg?label=stable)](https://github.com/JadeMatrix/SHOW/releases/latest)
[![latest version](https://img.shields.io/github/release/JadeMatrix/SHOW/all.svg?label=latest)](https://github.com/JadeMatrix/SHOW/releases)
[![Documentation Status](https://readthedocs.org/projects/show-cpp/badge/?version=v0.8.2)](http://show-cpp.readthedocs.io/en/v0.8.2/?badge=v0.8.2)

SHOW is an idiomatic library for standalone webserver applications written for modern C++.  SHOW is simple in the same way the standard library is simple — it doesn't make any design decisions for the programmer, instead offering a set of primitives for building an HTTP web application.  Everything — when to serve, what requests to accept, even whether to send a response at all — is completely up to the programmer.

SHOW assumes a modern approach to application hosting, and is intended to be run behind a full reverse proxy such as [NGINX](https://nginx.org/).  As such, SHOW will not support HTTP/2 or TLS (HTTPS); instead, you should write your applications to serve local HTTP/1.0 and HTTP/1.1 requests.

You can find SHOW's documentation, including a tutorial, on [ReadTheDocs.io](http://show-cpp.readthedocs.io/).  SHOW uses the [`zlib` license](LICENSE).  A C++11 compiler and a POSIX operating system are required.
