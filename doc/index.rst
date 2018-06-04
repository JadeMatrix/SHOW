.. SHOW documentation master file, created by sphinx-quickstart on Tue Dec 5
   16:02:43 2017.

Simple Header-Only Webserver
============================

SHOW is designed to be an idiomatic library for standalone webserver applications written for modern C++.  SHOW is simple in the same way the standard library is simple — it doesn't make any design decisions for the programmer, instead giving them a set of primitives for building an HTTP web application.

Both HTTP/1.0 and HTTP/1.1 are supported.  SHOW assumes a modern approach to application hosting, and is intended to be run behind a full reverse proxy such as `NGINX <https://nginx.org/>`_.  As such, SHOW will not support HTTP/2 or TLS (HTTPS).  Instead, you should write your applications to serve local HTTP/1.0 and HTTP/1.1 requests.

SHOW uses the `zlib license <https://github.com/JadeMatrix/SHOW/blob/master/LICENSE>`_.  C++11 support and a POSIX operating system (or POSIX compatibility layer) are required.

.. toctree::
    :maxdepth: 2
    :caption: Contents:
    
    Tutorial
    Classes
    Functions
    Constants
    Utilities

Indices and tables
==================

* :ref:`genindex`
* :ref:`search`
