=======
Request
=======

.. cpp:class:: show::request : public std::streambuf
    
    Represents a single request sent by a client.  Inherits from :cpp:class:`std::streambuf`, so it can be used as-is or with a :cpp:class:`std::istream`.
    
    .. seealso::
        
        * :cpp:type:`std::streambuf` on `cppreference.com <http://en.cppreference.com/w/cpp/io/basic_streambuf>`_
        
        * :cpp:type:`std::istream` on `cppreference.com <http://en.cppreference.com/w/cpp/io/basic_istream>`_
    
    .. cpp:enum:: show::request::content_length_flag_type
        
        A utility type for :cpp:member:`show::request::unknown_content_length()` with the values:
        
        +-----------+--------------+
        | Value     | Evaluates to |
        +===========+==============+
        | ``NO``    | ``false``    |
        +-----------+--------------+
        | ``YES``   | ``true``     |
        +-----------+--------------+
        | ``MAYBE`` | ``true``     |
        +-----------+--------------+
    
    .. cpp:member:: const std::string& show::request::client_address
        
        The IP address of the client that sent the request
    
    .. cpp:member:: const unsigned int& show::request::client_port
        
        The port of the client that sent the request
    
    .. cpp:function:: bool show::request::eof() const
        
        Returns whether or not the request, acting as a :cpp:class:`std::streambuf`, has reached the end of the request contents.  Always returns ``false`` if the content length is unknown.
        
        .. seealso::
            
            * :cpp:member:`show::request::unknown_content_length`
    
    .. cpp:function:: show::request::request( show::connection& )
        
        Constructs a new request on a connection.  Blocks until a connection is sent, the connection timeout is reached, or the client disconnects.  May also throw :cpp:class:`show::request_parse_error` if the data sent by the client cannot be understood as an HTTP request.
        
        .. seealso::
            
            * :cpp:class:`show::connection`
            
            * :cpp:class:`show::connection_timeout`
            
            * :cpp:class:`show::client_disconnected`
            
            * :cpp:class:`show::request_parse_error`
    
    .. cpp:function:: show::request::request( show::request&& )
        
        Explicit `move constructor`_ as one can't be generated for this class
        
        .. _move constructor: http://en.cppreference.com/w/cpp/language/move_constructor
    
    .. cpp:member:: const show::http_protocol& show::request::protocol
        
        The HTTP protocol used by the request.  If ``NONE``, it's usually safe to assume HTTP/1.0.  If ``UNKNOWN``, typically either a *400 Bad Request* should be returned, just assume HTTP/1.0 to be permissive, or try to interpret something from :cpp:member:`show::request::protocol_string`.
        
        .. seealso::
            
            * :cpp:type:`show::http_protocol`
            
            * :cpp:member:`show::request::protocol_string`
    
    .. cpp:member:: const std::string& show::request::protocol_string
        
        The raw protocol string sent in the request, useful if :cpp:member:`show::request::protocol` is ``UNKNOWN``
    
    .. cpp:member:: const std::string& show::request::method
        
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
    
    .. cpp:member:: const std::std::vector< std::string >& show::request::path
        
        The request path separated into its elements, each of which has been URL- or percent-decoded.  For example::
            
            /foo/bar/hello+world/%E3%81%93%E3%82%93%E3%81%AB%E3%81%A1%E3%81%AF
        
        becomes::
            
            {
                "foo",
                "bar"
                "hello world",
                "こんにちは"
            }
    
    .. cpp:member:: const show::query_args_t& show::request::query_args
        
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
        
        .. seealso::
            
            * :cpp:type:`show::query_args_t`
    
    .. cpp:member:: const show::headers_t& show::request::headers
        
        The request headers
        
        .. seealso::
            
            * :cpp:type:`show::headers_t`
            
            * `List of common HTTP headers on Wikipedia <https://en.wikipedia.org/wiki/List_of_HTTP_header_fields>`_
    
    .. cpp:member:: const show::request::content_length_flag_type& show::request::unknown_content_length
        
        Whether the content length of the request could be interpreted
        
        This member may be a bit confusing because it is "*un*-known" rather than "know".  It's convenient for :cpp:type:`show::request::content_length_flag_type` to evaluate to a boolean value, but there are two possible reasons the content length would be unknown.  Either the request did not send a *Content-Length* header, or the value supplied is not an integer.  In many languages (including C++), 0 is ``false`` and any other value is ``true``; so the boolean value needs to be ``false`` for a known content length and ``true`` for anything else.
        
        .. seealso::
            
            * :cpp:type:`show::request::content_length_flag_type`
    
    .. cpp:member:: unsigned long long show::request::content_length
        
        The number of bytes in the request content; only holds a meaningful value if :cpp:member:`show::request::unknown_content_length` is ``YES``/``true``
        
        .. seealso::
            
            * :cpp:member:`show::request::unknown_content_length`
