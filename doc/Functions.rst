=========
Functions
=========

.. cpp:namespace-push:: show

.. cpp:function:: std::string url_encode( const std::string& o, bool use_plus_space = true )
    
    URL-encode a string ``o``, escaping all reserved, special, or non-ASCII characters with `percent-encoding <https://en.wikipedia.org/wiki/Percent-encoding>`_.
    
    If ``use_plus_space`` is ``true``, spaces will be replaced with ``+`` rather than ``%20``.

.. cpp:function:: std::string url_decode( const std::string& )
    
    Decode a `URL- or percent-encoded <https://en.wikipedia.org/wiki/Percent-encoding>`_ string.  Throws :cpp:class:`url_decode_error` if the input string is not validly encoded.

.. cpp:function:: string show::base64_encode( const std::string& o, const char* chars = base64_chars_standard )
    
    `Base-64 <https://en.wikipedia.org/wiki/Base64>`_ a string ``o`` using the character set ``chars``, which must point to a ``char`` array of length 64.
    
    .. seealso::
        
        * :cpp:var:`base64_chars_standard`
        
        * :cpp:var:`base64_chars_urlsafe`

.. cpp:function:: std::string base64_decode( const std::string& o, const char* chars = base64_chars_standard )
    
    Decode a `base-64 <https://en.wikipedia.org/wiki/Base64>`_ encoded string ``o`` using the character set ``chars``, which must point to a ``char`` array of length 64.  Throws a :cpp:class:`base64_decode_error` if the input is not encoded against ``chars`` or has incorrect padding.
    
    .. seealso::
        
        * :cpp:var:`base64_chars_standard`
        
        * :cpp:var:`base64_chars_urlsafe`
