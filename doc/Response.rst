========
Response
========

.. cpp:namespace-push:: show

.. cpp:class:: response : public std::streambuf
    
    Represents a single response to a request.  Inherits from :cpp:class:`std::streambuf`, so it can be used as-is or with a :cpp:class:`std::ostream`.
    
    SHOW does not prevent mutliple response from being created or sent for a single request.  Most of the time this is something that would break the application; however, under certain conditions in HTTP/1.1 multiple *100*-type responses can be sent before a final *200+* response.
    
    .. seealso::
        
        * :cpp:type:`std::streambuf` on `cppreference.com <http://en.cppreference.com/w/cpp/io/basic_streambuf>`_
        
        * :cpp:type:`std::ostream` on `cppreference.com <http://en.cppreference.com/w/cpp/io/basic_ostream>`_
    
    .. cpp:function:: response( connection&, protocol, const response_code&, const headers_t& )
        
        Constructs a new response to the client who made a connection.  The protocols, response code, and headers are immediately buffered and cannot be changed after the response is created, so they have to be passed to the constructor.
    
    .. cpp:function:: ~response()
        
        Destructor for a response object; ensures the response is flushed
    
    .. cpp:function:: virtual void flush()
        
        Ensure the content currently written to the request is sent to the client
