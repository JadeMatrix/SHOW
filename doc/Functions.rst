=========
Functions
=========

.. cpp:namespace-push:: show

.. cpp:function:: std::string url_encode( const std::string& o, internal::flags< url_flags > = url_flags::USE_PLUS_SPACE )
    
    URL-encode a string ``o``, escaping all reserved, special, or non-ASCII characters with `percent-encoding <https://en.wikipedia.org/wiki/Percent-encoding>`_.
    
    If ``{}`` is passed as the flags, spaces will be replaced with ``%20`` rather than ``+``.

.. cpp:function:: std::string url_decode( const std::string& )
    
    Decode a `URL- or percent-encoded <https://en.wikipedia.org/wiki/Percent-encoding>`_ string.  Throws :cpp:class:`url_decode_error` if the input string is not validly encoded.
