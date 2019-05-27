=====
Types
=====

Main Types
==========

The public interfaces to the main SHOW classes are documented on the following pages:

.. toctree::
    :maxdepth: 2
    
    Server
    Connection
    Request
    Response

Support Types
=============

.. cpp:namespace-push:: show

.. cpp:enum-class:: protocol
    
    Symbolizes the HTTP protocols understood by SHOW.  Possible enum values are:
    
    +--------------+---------------------------------------------------------+
    | ``http_1_0`` | HTTP/1.0                                                |
    +--------------+---------------------------------------------------------+
    | ``http_1_1`` | HTTP/1.1                                                |
    +--------------+---------------------------------------------------------+
    | ``none``     | The request did not specify a protocol version          |
    +--------------+---------------------------------------------------------+
    | ``unkown``   | The protocol specified by the request wasn't recognized |
    +--------------+---------------------------------------------------------+
    
    There is no ``http_2`` as SHOW is not intended to handle HTTP/2 requests.  These are much better handled by a reverse proxy such as `NGINX <https://wiki.nginx.org/>`_, which will convert them into HTTP/1.0 or HTTP/1.1 requests for SHOW.

.. cpp:class:: response_code
    
    A simple utility ``struct`` that encapsulates the numerical code and description for an HTTP status code.  An object of this type can easily be statically initialized like so::
        
        show::response_code rc = { 404, "Not Found" };
    
    See the `list of HTTP status codes <https://en.wikipedia.org/wiki/List_of_HTTP_status_codes>`_ on Wikipedia for an easy reference for the standard code & description values.
    
    The two fields are defined as:
    
    .. cpp:member:: unsigned short code
    
    .. cpp:member:: std::string description

.. cpp:class:: query_args_type
    
    An alias for :cpp:class:`std::map\< std::string, std::vector\< std::string > >`, and can be statically initialized like one::
        
        show::query_args_type args{
            { "tag", { "foo", "bar" } },
            { "page", { "3" } }
        };
    
    This creates a variable ``args`` which represents the query string ``?tag=foo&tag=bar&page=3``.
    
    .. seealso::
        
        * :cpp:type:`std::map` on `cppreference.com <http://en.cppreference.com/w/cpp/container/map>`_
        
        * :cpp:type:`std::vector` on `cppreference.com <http://en.cppreference.com/w/cpp/container/vector>`_

.. cpp:class:: headers_type
    
    An alias for :cpp:class:`std::map\< std::string, std::vector\< std::string >, show::internal::less_ignore_case_ASCII >`, where :cpp:class:`show::internal::less_ignore_case_ASCII` is a case-insensitive `compare <http://en.cppreference.com/w/cpp/container/map>`_ for :cpp:class:`std::map`.
    
    While HTTP header names are typically given in ``Dashed-Title-Case``, they are technically case-insensitive.  Additionally, in general a given header name may appear more than once in a request or response.  This type satisfies both these constraints.
    
    Headers can be statically initialized::
        
        show::headers_type headers{
            { "Content-Type", { "text/plain" } },
            { "Set-Cookie", {
                "cookie1=foobar",
                "cookie2=SGVsbG8gV29ybGQh"
            } }
        };
    
    .. seealso::
        
        * :cpp:type:`std::map` on `cppreference.com <http://en.cppreference.com/w/cpp/container/map>`_
        
        * :cpp:type:`std::vector` on `cppreference.com <http://en.cppreference.com/w/cpp/container/vector>`_

.. cpp:enum-class:: url_flags
    
    Options that can be passed to :cpp:func:`url_encode`.  Possible enum values are:
    
    +--------------------+--------------------------------------------+
    | ``use_plus_space`` | Encode spaces as ``+`` rather than ``%20`` |
    +--------------------+--------------------------------------------+

Throwables
==========

Not all of these strictly represent an error state when throw; some signal common situations that should be treated very much in the same way as exceptions.  SHOW's throwables are broken into two categories — connection interruptions and exceptions.

Connection interruptions
------------------------

.. cpp:class:: connection_interrupted
    
    A common base class for both types of connection interruptions.  Note that this does not inherit from :cpp:type:`std::exception`.

.. cpp:class:: connection_timeout : public connection_interrupted
    
    An object of this type will be thrown in two general situations:
    
    * A server object timed out waiting for a new connection
    * A connection, request, or response timed out reading from or sending to a client
    
    In the first situation, generally the application will simply loop and start waiting again.  In the second case, the application may want to close the connection or continue waiting with either the same timoute or some kind of falloff.  Either way the action will be application-specific.

.. cpp:class:: client_disconnected : public connection_interrupted
    
    This is thrown when SHOW detects that a client has broken connection with the server and no further communication can occur.

Exceptions
----------

.. seealso::
    
    * :cpp:type:`std::runtime_error` on `cppreference.com <http://en.cppreference.com/w/cpp/error/runtime_error/runtime_error>`_

.. cpp:class:: socket_error : public std::runtime_error
    
    An unrecoverable, low-level error occurred inside SHOW.  If thrown while handling a connection, the connection will no longer be valid but the server should be fine.  If thrown while creating or working with a server, the server object itself is in an unrecoverable state and can no longer serve.
    
    The nature of this error when thrown by a server typically implies trying again will not work.  If the application is designed to serve on a single IP/port, you will most likely want to exit the program with an error.

.. cpp:class:: request_parse_error : public std::runtime_error
    
    Thrown when creating a request object from a connection and SHOW encounters something it can't manage to interpret into a :cpp:class:`request`.
    
    As parsing the offending request almost certainly failed midway, garbage data will likely in the connection's buffer.  Currently, the only safe way to handle this exception is to close the connection.

.. cpp:class:: response_marshall_error : public std::runtime_error
    
    Thrown by :cpp:class:`response`'s constructor when the response arguments cannot be marshalled into a valid HTTP response:
    
    * One of the header names is an empty string
    * One of the header names contains a character other than A-Z, a-z, 0-9, or -
    * Any header value is an empty string

.. cpp:class:: url_decode_error : public std::runtime_error
    
    Thrown by :cpp:func:`url_decode()` when the input is not a valid `URL- or percent-encoded <https://en.wikipedia.org/wiki/Percent-encoding>`_ string.
    
    .. note::
        :cpp:func:`url_encode()` shouldn't throw an exception, as any string can be converted to percent-encoding.
