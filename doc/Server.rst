======
Server
======

.. cpp:namespace-push:: show

.. cpp:class:: server
    
    The server class serves as the basis for writing an HTTP application with SHOW.  Creating a server object allows the application to handle HTTP requests on a single IP/port combination.
    
    .. cpp:function:: server( const std::string& address, port_type port, int timeout = -1 )
        
        Constructs a new server to serve on the given IP address and port.  The IP address will typically be ``localhost``/``0.0.0.0``/``::``.
        
        The port should be some random higher-level port chosen for the application.  Alternatively, a port value of 0 signifies "any free port," in which case :cpp:func:`port` can be used to obtain the port chosen by the operating system.
        
        The timeout is the maximum number of seconds :cpp:func:`serve()` will wait for an incoming connection before throwing :cpp:class:`connection_timeout`.  A value of 0 means that :cpp:func:`serve()` will return immediately if there are no connections waiting to be served; -1 means :cpp:func:`serve()` will wait forever (until the program is interrupted).
    
    .. cpp:function:: ~server()
        
        Destructor for a server; any existing connections made from this server will continue to function.
    
    .. cpp:function:: connection serve()
        
        Either returns the next connection waiting to be served or throws :cpp:class:`connection_timeout`.
    
    .. cpp:function:: const std::string& address() const
        
        Get the address this server is servering on
    
    .. cpp:function:: port_type port() const
        
        Get the port this server is servering on
    
    .. cpp:function:: int timeout() const
        
        Get the current timeout of this server
    
    .. cpp:function:: int timeout( int )
        
        Set the timeout of this server to a number of seconds, 0, or -1
