========
Tutorial
========

This shows the basic usage of SHOW; see the `examples <https://github.com/JadeMatrix/SHOW/tree/master/examples>`_ for a more thorough introduction.

Including & Compiling
=====================

For GCC and Clang, you can either link `show.hpp` to one of your standard include search paths, or use the ``-I`` flag to tell the compiler where too find the header::
    
    clang++ -I "SHOW/src/" ...

SHOW is entirely contained in a single header file, you have to do then is include SHOW using ``#include <show.hpp>``.  With either compiler you'll also need to specify C++11 support with ``-std=c++11``.

If you use `CMake <https://cmake.org/>`_ and don't have SHOW linked to said include path, you'll need to include the following in your *CMakeLists.txt*::
    
    include_directories( "SHOW/src/" )

replacing ``"SHOW/src/"`` with wherever you've cloned or installed SHOW.  Switch to C++11 mode with::
    
    set( CMAKE_CXX_STANDARD 11 )
    set( CMAKE_CXX_STANDARD_REQUIRED ON )

Creating a Server
=================

To start serving requests, first create a server object::
    
    show::server my_server(
        "0.0.0.0",  // IP address on which to serve
        9090,       // Port on which to serve
    );

That's it, you've made a server that sits there forever until it gets a connection, then hangs.  Not terribly useful, but that's easy to fix.

Handling a Connection
=====================

For each call of ``my_server.serve()`` a single :cpp:class:`show::connection` object will be returned or a :cpp:class`show::connection_timeout` thrown. You may want to use something like this::
    
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

The server listen timeout can be a positive number, 0, or -1. If it is -1, the server will continue listening until interrupted by a signal; if 0, :cpp:member:`serve()` will throw a :cpp:class:`show::connection_timeout` immediately unless connections are available.

The connection is now independent from the server. You can adjust the connection's timeout independently using :cpp:func:`show::connection::timeout()`.  You can also pass it off to a worker thread for processing so your server can continue accepting other connections; this is usually how you'd implement a real web application.

Reading Requests
================

:cpp:class:`show::request` objects have a number of ``const`` fields containing the HTTP request's metadata; you can see descriptions of them all in the docs for the class.

Note that these fields do not include the request content, if any. This is because HTTP allows the request content to be streamed to the server. In other words, the server can interpret the headers then wait for the client to send data over a period of time. For this purpose, :cpp:class`show::request` inherits from :cpp:class:`std::streambuf`, implementing the read/get functionality. You can use the raw :cpp:class:`std::streambuf` methods to read the incoming data, or create a :cpp:class:`std::istream` from the request object for :cpp:var:`std::cin`-like behavior.

For example, if your server is expecting the client to *POST* a single integer, you can use::
    
    show::request request( test_server.serve() );
    
    std::istream request_content_stream( request );
    
    int my_integer;
    request_content_stream >> my_integer;

Please note that the above is not terribly safe; production code should include various checks to guard against buggy or malignant clients.

Also note that individual request operations may timeout, so the entire serve code should look like this::
    
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
            catch( const show::connection_timeout& ct )
            {
                std::cout << "got a request, but client disconnected!\n";
            }
            catch( const show::connection_timeout& ct )
            {
                std::cout << "got a request, but client timed out!\n";
            }
        }
        catch( const show::connection_timeout& ct )
        {
            std::cout
                << "timed out waiting for a connection, looping..."
                << std::endl
            ;
            continue;
        }

If this feels complicated, it is.  Network programming like this reveals the worst parts of distributed programming, and there's a lot that can go wrong between the client and the server.

.. seealso::
    
    * :cpp:class:`std::streambuf` on `cppreference.com <http://en.cppreference.com/w/cpp/io/basic_streambuf>`_
    
    * :cpp:class:`std::istream` on `cppreference.com <http://en.cppreference.com/w/cpp/io/basic_istream>`_
    
    * :cpp:var:`std::cin` on `cppreference.com <http://en.cppreference.com/w/cpp/io/cin>`_

Sending Responses
=================

Sending responses is slightly more complex than reading basic requests, aside from the error handling which should wrap both.

Say you want to send a "Hello World" message for any incoming request. First, start with a string containing the response message::
    
    std::string response_content = "Hello World";

Next, create a headers object to hold the content type and length headers (note that header values must be strings)::
    
    show::headers_t headers = {
        { "Content-Type", { "text/plain" } },
        { "Content-Length", {
            std::to_string( response_content.size() )
        } }
    };

Since it's a :cpp:class:`std::map`, you can also add headers to a :cpp:type:`show::headers_t` like this::
    
    headers[ "Content-Type" ].push_back( "text/plain" );

Then, set the `HTTP status code <https://en.wikipedia.org/wiki/List_of_HTTP_status_codes>`_ for the response to the generic *200 OK*::
    
    show::response_code code = {
        200,
        "OK"
    };

Creating a response object requires the headers and response code to have been decided already, as they are marshalled (serialized) and buffered for sending as soon as the object is created. A response object also needs to know which request it is in response to. While there's nothing preventing you from creating multiple responses to a single request this way, most of the time that will break your application.

Create a response like this::
    
    show::response response(
        request,
        show::http_protocol::HTTP_1_0,
        code,
        headers
    );

Finally, send the response content. Here, a :cpp:class:`std::ostream` is used, as :cpp:class:`show::response` inherits from and implements the write/put functionality of :cpp:class:`std::streambuf`::
    
    std::ostream response_stream( &response );
    response_stream << response_content;

.. seealso::
    
    * :cpp:class:`std::map` on `cppreference.com <http://en.cppreference.com/w/cpp/container/map>`_
    
    * :cpp:class:`std::ostream` on `cppreference.com <http://en.cppreference.com/w/cpp/io/basic_ostream>`_
    
    * :cpp:class:`std::streambuf` on `cppreference.com <http://en.cppreference.com/w/cpp/io/basic_streambuf>`_
