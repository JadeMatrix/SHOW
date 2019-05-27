=======
Request
=======

.. cpp:namespace-push:: show

.. cpp:class:: request : public std::streambuf
    
    Represents a single request sent by a client.  Inherits from :cpp:class:`std::streambuf`, so it can be used as-is or with a :cpp:class:`std::istream`.
    
    .. seealso::
        
        * :cpp:type:`std::streambuf` on `cppreference.com <http://en.cppreference.com/w/cpp/io/basic_streambuf>`_
        
        * :cpp:type:`std::istream` on `cppreference.com <http://en.cppreference.com/w/cpp/io/basic_istream>`_
    
    .. cpp:enum:: content_length_flag
        
        A utility type for :cpp:func:`unknown_content_length()` with the values:
        
        +-----------+--------------+
        | Value     | Evaluates to |
        +===========+==============+
        | ``no``    | ``false``    |
        +-----------+--------------+
        | ``yes``   | ``true``     |
        +-----------+--------------+
        | ``maybe`` | ``true``     |
        +-----------+--------------+
    
    .. cpp:function:: const std::string& client_address() const
        
        The IP address of the client that sent the request
    
    .. cpp:function:: const unsigned int client_port() const
        
        The port of the client that sent the request
    
    .. cpp:function:: bool eof() const
        
        Returns whether or not the request, acting as a :cpp:class:`std::streambuf`, has reached the end of the request contents.  Always returns ``false`` if the content length is unknown.
        
        .. seealso::
            
            * :cpp:func:`unknown_content_length`
    
    .. cpp:function:: request( connection& )
        
        Constructs a new request on a connection.  Blocks until a connection is sent, the connection timeout is reached, or the client disconnects.  May also throw :cpp:class:`request_parse_error` if the data sent by the client cannot be understood as an HTTP request.
        
        .. seealso::
            
            * :cpp:class:`connection_timeout`
            
            * :cpp:class:`client_disconnected`
    
    .. cpp:function:: request( request&& )
        
        Explicit `move constructor`_ as one can't be generated for this class
        
        .. _move constructor: http://en.cppreference.com/w/cpp/language/move_constructor
    
    .. cpp:function:: void flush()
        
        Flushes the request contents from the buffer, putting it in a state where the next request can be extracted.  It is only safe to call this function if :cpp:func:`unknown_content_length()` evaluates to ``false``.
    
    .. cpp:function:: protocol protocol() const
        
        The HTTP protocol used by the request.  If ``protocol::none``, it's usually safe to assume HTTP/1.0.  If ``protocol::unknown``, typically either a *400 Bad Request* should be returned, just assume HTTP/1.0 to be permissive, or try to interpret something from :cpp:func:`protocol_string`.
    
    .. cpp:function:: const std::string& protocol_string() const
        
        The raw protocol string sent in the request, useful if :cpp:func:`protocol` is ``protocol::unknown``
    
    .. cpp:function:: const std::string& method() const
        
        The request method as a capitalized ASCII string.  While the HTTP protocol technically does not restrict the available methods, typically this will be one of the following:
        
        +-------------+-----------------------------+
        | ``GET``     | Common methods              |
        +-------------+                             |
        | ``POST``    |                             |
        +-------------+                             |
        | ``PUT``     |                             |
        +-------------+                             |
        | ``DELETE``  |                             |
        +-------------+-----------------------------+
        | ``OPTIONS`` | Useful for APIs             |
        +-------------+-----------------------------+
        | ``PATCH``   | Relatively uncommon methods |
        +-------------+                             |
        | ``TRACE``   |                             |
        +-------------+                             |
        | ``HEAD``    |                             |
        +-------------+                             |
        | ``CONNECT`` |                             |
        +-------------+-----------------------------+
        
        .. seealso::
            
            * `List of common HTTP methods on Wikipedia <https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol#Request_methods>`_ for descriptions of the methods
    
    .. cpp:function:: const std::vector< std::string >& path() const
        
        The request path separated into its elements, each of which has been URL- or percent-decoded.  For example::
            
            /foo/bar/hello+world/%E3%81%93%E3%82%93%E3%81%AB%E3%81%A1%E3%81%AF
        
        becomes::
            
            {
                "foo",
                "bar"
                "hello world",
                "こんにちは"
            }
    
    .. cpp:function:: const query_args_type& query_args() const
        
        The request query arguments.  SHOW is very permissive in how it parses query arguments:
        
        +----------------------+----------------------------------------------------+
        | Query string         | Interpreted as                                     |
        +======================+====================================================+
        | ``?foo=1&bar=2``     | ``{ { "foo", { "1" } }, { "bar", { "2" } } }``     |
        +----------------------+----------------------------------------------------+
        | ``?foo=bar=baz``     | ``{ { "foo", { "baz" } }, { "bar", { "baz" } } }`` |
        +----------------------+----------------------------------------------------+
        | ``?foo=&bar=baz``    | ``{ { "foo", { "" } }, { "bar", { "baz" } } }``    |
        +----------------------+----------------------------------------------------+
        | ``?foo&bar=1&bar=2`` | ``{ { "foo", { "" } }, { "bar", { "1", "2" } } }`` |
        +----------------------+----------------------------------------------------+
    
    .. cpp:function:: const headers_type& headers() const
        
        The request headers
        
        .. seealso::
            
            * `List of common HTTP headers on Wikipedia <https://en.wikipedia.org/wiki/List_of_HTTP_header_fields>`_
    
    .. cpp:function:: content_length_flag unknown_content_length() const
        
        Whether the content length of the request could be interpreted
        
        This member may be a bit confusing because it is "*un*-known" rather than "know".  It's convenient for :cpp:type:`content_length_flag` to evaluate to a boolean value, but there are two possible reasons the content length would be unknown.  Either
        
        1. the request did not send a *Content-Length* header, or
        2. the value supplied is not an integer or multiple *Content-Length* headers were sent.
        
        In many languages (including C++), 0 is ``false`` and any other value is ``true``; so the boolean value needs to be ``false`` for a known content length and ``true`` for anything else.
    
    .. cpp:function:: unsigned long long content_length() const
        
        The number of bytes in the request content; only holds a meaningful value if :cpp:func:`unknown_content_length` is ``yes``/``true``
