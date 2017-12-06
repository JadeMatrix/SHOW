==========
Connection
==========

.. cpp:class:: show::connection
    
    
    
    .. cpp:function:: show::connection::connection( show::connection&& )
        
        Explicit `move constructor`_ as one can't be generated for :cpp:class:`connection`
        
        .. _move constructor: http://en.cppreference.com/w/cpp/language/move_constructor
    
    .. cpp:function:: show::connection::~connection()
        
        
    
    .. cpp:function:: const std::string& show::connection::client_address()
        
        
    
    .. cpp:function:: const unsigned int& show::connection::client_port()
        
        
    
    .. cpp:function:: int show::connection::timeout()
        
        
    
    .. cpp:function:: int show::connection::timeout( int )
        
        
