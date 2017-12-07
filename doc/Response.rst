========
Response
========

.. cpp:class:: show::response : public std::streambuf
    
    Represents a single response to a request.  Inherits from :cpp:class:`std::streambuf`, so it can be used as-is or with a :cpp:class:`std::ostream`.
    
    SHOW does not prevent mutliple response from being created or sent for a single request.  Most of the time this is something that would break the application; however, under certain conditions in HTTP/1.1 multiple *100*-type responses can be sent before a final *200+* response.
    
    .. seealso::
        
        * :cpp:type:`std::streambuf` on `cppreference.com <http://en.cppreference.com/w/cpp/io/basic_streambuf>`_
        
        * :cpp:type:`std::ostream` on `cppreference.com <http://en.cppreference.com/w/cpp/io/basic_ostream>`_
    
    .. cpp:function:: show::response::response( show::request&, show::http_protocol, const show::response_code&, const show::headers_t& )
        
        Constructs a new response in response to a request.  The protocols, response code, and headers are immediately buffered and cannot be changed after the response is created, so they have to be passed to the constructor.
        
        .. seealso::
            
            * :cpp:class:`show::request`
            
            * :cpp:type:`show::http_protocol`
            
            * :cpp:type:`show::response_code`
            
            * :cpp:type:`show::headers_t`
    
    .. cpp:function:: show::response::~response()
        
        Destructor for a response object; ensures the response is flushed
    
    .. cpp:function:: virtual void flush()
        
        Ensure the content currently written to the request is sent to the client
