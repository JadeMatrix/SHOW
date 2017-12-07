==================
Separate Utilities
==================

These are some useful utilities included with SHOW, but in their own header files so they're optional.

Base-64 Encoding
================

These are included in *show/base64.hpp*:

.. cpp:namespace-push:: show

.. cpp:function:: string base64_encode( const std::string& o, const char* chars = base64_chars_standard )
    
    `Base-64 <https://en.wikipedia.org/wiki/Base64>`_ a string ``o`` using the character set ``chars``, which must point to a ``char`` array of length 64.
    
    .. seealso::
        
        * :cpp:var:`base64_chars_standard`
        
        * :cpp:var:`base64_chars_urlsafe`

.. cpp:function:: std::string base64_decode( const std::string& o, const char* chars = base64_chars_standard )
    
    Decode a `base-64 <https://en.wikipedia.org/wiki/Base64>`_ encoded string ``o`` using the character set ``chars``, which must point to a ``char`` array of length 64.  Throws a :cpp:class:`base64_decode_error` if the input is not encoded against ``chars`` or has incorrect padding.
    
    .. seealso::
        
        * :cpp:var:`base64_chars_standard`
        
        * :cpp:var:`base64_chars_urlsafe`

.. cpp:class:: base64_decode_error : exception
    
    Thrown by :cpp:func:`base64_decode()` when the input is not valid `base-64 <https://en.wikipedia.org/wiki/Base64>`_.
    
    .. note::
        :cpp:func:`base64_encode()` shouldn't throw an exception, as any string can be converted to base-64.

.. cpp:var:: char* base64_chars_standard
    
    The standard set of `base-64 characters`_ for use with :cpp:func:`base64_encode()` and :cpp:func:`base64_decode()`
    
    .. _base-64 characters: https://en.wikipedia.org/wiki/Base64

.. cpp:var:: char* base64_chars_urlsafe
    
    The URL_safe set of `base-64 characters`_ for use with :cpp:func:`base64_encode()` and :cpp:func:`base64_decode()`, making the following replacements:
    
    .. _base-64 characters: https://en.wikipedia.org/wiki/Base64
    
    * ``+`` → ``-``
    * ``/`` → ``_``
