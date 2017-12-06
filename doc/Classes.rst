===============
Classes & Types
===============

Classes
=======

The public interfaces to the main SHOW classes are outlined in the following documents:

.. toctree::
    :maxdepth: 2
    
    Server
    Connection
    Request
    Response

Types
=====

.. cpp:enum:: show::http_protocol
    
    ``NONE``, ``UNKOWN``, ``HTTP_1_0``, ``HTTP_1_1``; there is no ``HTTP_2`` as HTTP/2 is supported at the reverse proxy level

.. cpp:class:: show::response_code
    
    

.. cpp:class:: show::query_args_t
    
    

.. cpp:class:: show::headers_t
    
    
Throwables
==========

Not all of these are strictly exceptions.

Connection interruptions
------------------------

.. cpp:class:: show::connection_timeout
    
    

.. cpp:class:: show::client_disconnected
    
    

Exceptions
----------

.. cpp:class:: show::exception : std::exception
    
    

.. cpp:class:: show::socket_error : show::exception
    
    

.. cpp:class:: show::request_parse_error : show::exception
    
    

.. cpp:class:: show::response_marshall_error : show::exception
    
    

.. cpp:class:: show::url_decode_error : show::exception
    
    

.. cpp:class:: show::base64_decode_error : show::exception
    
    

