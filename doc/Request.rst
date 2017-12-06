=======
Request
=======

.. cpp:class:: show::request : public std::streambuf
    
    
    
    .. cpp:enum:: show::request::content_length_flag_type
        
        ``NO`` (``false``), ``YES``, and ``MAYBE`` (both ``true``)
    
    .. cpp:member:: const std::string& show::request::client_address
        
        
    
    .. cpp:member:: const unsigned int& show::request::client_port
        
        
    
    .. cpp:function:: bool show::request::eof() const
        
        
    
    .. cpp:function:: show::request::request( show::connection& )
        
        
    
    .. cpp:function:: show::request::request( show::request&& )
        
        Explicit `move constructor`_ as one can't be generated for :cpp:class:`request`
        
        .. _move constructor: http://en.cppreference.com/w/cpp/language/move_constructor
    
    .. cpp:member:: const show::http_protocol& show::request::protocol
        
        .. seealso::
            
            :cpp:type:`show::http_protocol`
    
    .. cpp:member:: const std::string& show::request::protocol_string
        
        
    
    .. cpp:member:: const std::string& show::request::method
        
        
    
    .. cpp:member:: const std::std::vector< std::string >& show::request::path
        
        
    
    .. cpp:member:: const show::query_args_t& show::request::query_args
        
        .. seealso::
            
            :cpp:type:`show::query_args_t`
    
    .. cpp:member:: const show::headers_t& show::request::headers
        
        .. seealso::
            
            :cpp:type:`show::headers_t`
    
    .. cpp:member:: const show::request::content_length_flag_type& show::request::unknown_content_length
        
        .. seealso::
            
            :cpp:type:`show::request::content_length_flag_type`
    
    .. cpp:member:: unsigned long long show::request::content_length
        
        
