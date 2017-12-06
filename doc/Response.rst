========
Response
========

.. cpp:class:: show::response : public std::streambuf
    
    
    
    .. cpp:function:: show::response::response( show::request&, show::http_protocol, const show::response_code&, const headers_t& )
        
        
    
    .. cpp:function:: show::response::~response()
        
        Destructor for a response object; ensures the response is flushed.
    
    .. cpp:function:: virtual void flush()
        
        
