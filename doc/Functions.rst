=========
Functions
=========

.. cpp:namespace-push:: show

.. cpp:function:: std::string url_encode( const std::string& o, bool use_plus_space = true )
    
    URL-encode a string ``o``, escaping all reserved, special, or non-ASCII characters with `percent-encoding <https://en.wikipedia.org/wiki/Percent-encoding>`_.
    
    If ``use_plus_space`` is ``true``, spaces will be replaced with ``+`` rather than ``%20``.

.. cpp:function:: std::string url_decode( const std::string& )
    
    Decode a `URL- or percent-encoded <https://en.wikipedia.org/wiki/Percent-encoding>`_ string.  Throws :cpp:class:`url_decode_error` if the input string is not validly encoded.
