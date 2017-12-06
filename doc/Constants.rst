=========
Constants
=========

All constants are `const`.

.. cpp:class:: show::version
    
    An anonymous ``struct`` containing information about the current SHOW version.  Has the following members:
    
    .. cpp:member:: std::string name
        
        The proper name of SHOW as it should appear referenced in headers, log messages, etc.
    
    .. cpp:member:: int major
        
        The major SHOW version (``X.0.0``)
    
    .. cpp:member:: int minor
        
        The minor SHOW version (``0.X.0``)
    
    .. cpp:member:: int revision
        
        The SHOW version revision (``0.0.X``)
    
    .. cpp:member:: std::string string
        
        A string representing the major, minor, and revision version numbers

.. cpp:var:: char* show::base64_chars_standard
    
    The standard set of `base-64 characters`_ for use with :cpp:func:`show::base64_encode`
    
    .. _base-64 characters: https://en.wikipedia.org/wiki/Base64

.. cpp:var:: char* show::base64_chars_urlsafe
    
    The URL_safe set of `base-64 characters`_ for use with :cpp:func:`show::base64_encode`, making the following replacements:
    
    .. _base-64 characters: https://en.wikipedia.org/wiki/Base64
    
    * ``+`` → ``-``
    * ``/`` → ``_``
