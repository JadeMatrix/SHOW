==================
Separate Utilities
==================

These are some useful utilities included with SHOW, but in their own header files so they're optional.

Base-64 Encoding
================

These are utilities for handling `base64 <https://en.wikipedia.org/wiki/Base64>`_-encoded strings, very commonly used for transporting binary data in web applications.  They are included in *show/base64.hpp*.

.. cpp:namespace-push:: show

.. cpp:function:: string base64_encode( const std::string& o, const char* chars = base64_chars_standard )
    
    Base64-encode a string ``o`` using the character set ``chars``, which must point to a ``char`` array of length 64.
    
    .. seealso::
        
        * :cpp:var:`base64_chars_standard`
        
        * :cpp:var:`base64_chars_urlsafe`

.. cpp:function:: std::string base64_decode( const std::string& o, const char* chars = base64_chars_standard )
    
    Decode a base64-encoded string ``o`` using the character set ``chars``, which must point to a ``char`` array of length 64.  Throws a :cpp:class:`base64_decode_error` if the input is not encoded against ``chars`` or has incorrect padding.
    
    .. seealso::
        
        * :cpp:var:`base64_chars_standard`
        
        * :cpp:var:`base64_chars_urlsafe`

.. cpp:class:: base64_decode_error : public std::runtime_error
    
    Thrown by :cpp:func:`base64_decode()` when the input is not valid base64.
    
    .. note::
        :cpp:func:`base64_encode()` shouldn't throw an exception, as any string can be converted to base-64.

.. cpp:var:: char* base64_chars_standard
    
    The standard set of base64 characters for use with :cpp:func:`base64_encode()` and :cpp:func:`base64_decode()`

.. cpp:var:: char* base64_chars_urlsafe
    
    The URL_safe set of base64 characters for use with :cpp:func:`base64_encode()` and :cpp:func:`base64_decode()`, making the following replacements:
    
    * ``+`` → ``-``
    * ``/`` → ``_``
