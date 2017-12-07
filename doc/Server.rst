======
Server
======

.. cpp:class:: show::server
    
    The server class serves as the basis for writing an HTTP application with SHOW.  Creating a server object allows the application to handle HTTP requests on a single IP/port combination.
    
    .. cpp:function:: show::server::server( const std::string& address, unsigned int port, int timeout = -1 )
        
        Constructs a new server to serve on the given IP address and port.  The IP address will typically be ``localhost``/``0.0.0.0``/``::``.  The port should be some random higher-level port chosen for the application.
        
        The timeout is the maximum number of seconds :cpp:func:`show::server::serve()` will wait for an incoming connection before throwing :cpp:class:`show::connection_timeout`.  A value of 0 means that :cpp:func:`show::server::serve()` will return immediately if there are no connections waiting to be served; -1 means :cpp:func:`show::server::serve()` will wait forever (until the program is interrupted).
    
    .. cpp:function:: show::server::~server()
        
        Destructor for a server; any existing connections made from this server will continue to function
    
    .. cpp:function:: show::connection show::server::serve()
        
        Either returns the next connection waiting to be served or throws :cpp:class:`show::connection_timeout`.
        
        .. seealso::
            
            :cpp:type:`show::connection`
    
    .. cpp:function:: const std::string& show::server::address() const
        
        Get the address this server is servering on
    
    .. cpp:function:: unsigned int show::server::port() const
        
        Get the port this server is servering on
    
    .. cpp:function:: int show::server::timeout() const
        
        Get the current timeout of this server
    
    .. cpp:function:: int show::server::timeout( int )
        
        Set the timeout of this server to a number of seconds, 0, or -1
