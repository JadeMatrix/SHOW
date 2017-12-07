==========
Connection
==========

.. cpp:class:: show::connection
    
    Objects of this type represent a connection between a single client and a server.  A connection object can be used to generate :cpp:class:`show::request` objects; one in the case of HTTP/1.0 or multiple in the case of HTTP/1.1.
    
    The connection class has no public constructor (besides the move constructor), and can only be created by calling :cpp:func:`show::server::serve()`.
    
    .. cpp:function:: show::connection::connection( show::connection&& )
        
        Explicit `move constructor`_ as one can't be generated for this class
        
        .. _move constructor: http://en.cppreference.com/w/cpp/language/move_constructor
    
    .. cpp:function:: show::connection::~connection()
        
        Destructor for a connection, which closes it; any requests or responses created on this connection can no longer be read from or written to
    
    .. cpp:member:: const std::string& show::connection::client_address
        
        The IP address of the connected client
    
    .. cpp:member:: const unsigned int& show::connection::client_port
        
        The port of the connected client
    
    .. cpp:function:: int show::connection::timeout()
        
        Get the current timeout of this connection, initially inherited from the server the connection is created from
    
    .. cpp:function:: int show::connection::timeout( int )
        
        Set the timeout of this connection independently of the server; the argument is a number of seconds, 0, or -1
        
        .. seealso::
            
            * :cpp:func:`show::server::timeout()`
