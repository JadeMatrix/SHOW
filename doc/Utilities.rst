==================
Separate Utilities
==================

These are some useful utilities included with SHOW, but in their own header files so they're optional.

.. cpp:namespace-push:: show

Base-64 Encoding
================

These are utilities for handling `base64 <https://en.wikipedia.org/wiki/Base64>`_-encoded strings, very commonly used for transporting binary data in web applications.  They are included in *show/base64.hpp*.

.. cpp:function:: string base64::encode( const std::string& o, const base64::dict_type& dict = base64::dict_standard )
    
    Base64-encode a string ``o`` using the character set ``dict``, which must point to a ``char`` array of length 64.
    
    .. seealso::
        
        * :cpp:var:`base64::dict_standard`
        
        * :cpp:var:`base64::dict_urlsafe`

.. cpp:function:: std::string base64::decode( const std::string& o, const base64::dict_type& dict = base64::dict_standard, show::internal::flags< base64::flags > f = {} )
    
    Decode a base64-encoded string ``o`` using the character set ``dict``, which must point to a ``char`` array of length 64.  Throws a :cpp:class:`base64::decode_error` if the input is not encoded against ``dict`` or has incorrect padding.
    
    Incorrect padding can be ignored by passing ``show::base64::flags::ignore_padding`` as the ``flags`` argument.
    
    .. seealso::
        
        * :cpp:var:`base64::dict_standard`
        
        * :cpp:var:`base64::dict_urlsafe`

.. cpp:class:: base64::decode_error : public std::runtime_error
    
    Thrown by :cpp:func:`base64::decode()` when the input is not valid base64.
    
    .. note::
        :cpp:func:`base64::encode()` shouldn't throw an exception, as any string can be converted to base-64.

.. cpp:class:: base64::dict_type

    Type for defining base64 character dictionaries; alias for ``std::array< char, 64 >``.
    
    .. seealso::
        
        * :cpp:type:`std::array` on `cppreference.com <http://en.cppreference.com/w/cpp/container/array>`_

.. cpp:var:: base64::dict_type base64::dict_standard
    
    The standard set of base64 characters for use with :cpp:func:`base64::encode()` and :cpp:func:`base64::decode()`

.. cpp:var:: base64::dict_type base64::dict_urlsafe
    
    The URL_safe set of base64 characters for use with :cpp:func:`base64::encode()` and :cpp:func:`base64::decode()`, making the following replacements:
    
    * ``+`` → ``-``
    * ``/`` → ``_``

.. cpp:enum-class:: base64::flags
    
    Options that can be passed to :cpp:func:`base64::decode`.  Possible enum values are:
    
    +--------------------+-------------------------------------------------------------+
    | ``ignore_padding`` | Ignore missing padding on the end of base64-encoded strings |
    +--------------------+-------------------------------------------------------------+

Multipart Content Support
=========================

`Multipart content <https://en.wikipedia.org/wiki/MIME#Multipart_messages>`_ is used to send a number of data segments each with their own separate headers.  As such, text and binary data can be mixed in the same message.

SHOW provides the following utilities for parsing multipart requests in *show/multipart.hpp*.  Typically, the ``Content-Type`` header for these types of requests will look something like::
    
    Content-Type: multipart/form-data; boundary=AaB03x

The boundary string must be extracted from the header to pass to :cpp:class:`multipart`'s constructor.  A simple example with no error handling::
    
    const auto& header_value = request.headers()[ "Content-Type" ][ 0 ];
    auto content_supertype = header_value.substr( 0, header_value.find( "/" ) )
    if( content_supertype == "multipart" )
    {
        show::multipart parser{
            request,
            header_value.substr( header_value.find( "boundary=" ) + 9 )
        };
        
        // Iterate over multipart data ...
    }
    else
        // Process data as single message ...

.. cpp:class:: multipart
    
    class description
    
    .. cpp:function:: template< class String > multipart( std::streambuf& buffer, String&& boundary )
        
        Constructs a new multipart content parser.
        
        The supplied buffer will typically be a :cpp:class:`request` object, but because multipart content can contain other multipart content recursively it can also be a :cpp:class:`show::multipart::segment`.  The ``boundary`` variable is a `perfectly-forwarded <http://en.cppreference.com/w/cpp/utility/forward>`_ boundary string for the multipart data.
        
        Throws :cpp:class:`std::invalid_argument` if the boundary is an empty string.
        
        .. seealso::
            
            * :cpp:class:`std::invalid_argument` on `cppreference.com <en.cppreference.com/w/cpp/error/invalid_argument>`_
    
    .. cpp:function:: multipart::iterator begin()
        
        Returns an iterator pointing to the first segment in the multipart content.  Calling this more than once on the same :cpp:class:`multipart` throws a :cpp:class:`std::logic_error`.
        
        .. seealso::
            
            * :cpp:class:`std::logic_error` on `cppreference.com <en.cppreference.com/w/cpp/error/logic_error>`_
    
    .. cpp:function:: multipart::iterator end()
        
        Returns an iterator representing the end of the multipart content.
    
    .. cpp:function:: const std::string& boundary()
        
        The boundary string the :cpp:class:`multipart` is using to split the content
    
    .. cpp:function:: const std::streambuf& buffer()
        
        The buffer the :cpp:class:`multipart` is reading from

.. cpp:class:: multipart::iterator
    
    Iterator type for iterating over multipart data segments.  Implements most of `input iterator functionality <http://en.cppreference.com/w/cpp/concept/InputIterator>`_, except that its ``value_type`` (:cpp:class:`multipart::segment`) cannot be copied.

.. cpp:class:: multipart::segment : public std::streambuf
    
    Represents a segment of data in the multipart content being iterated over.  Cannot be copied.
    
    .. cpp:function:: const headers_type& headers()
        
        The headers for this individual segment of data; does not include the request's headers.

.. cpp:class:: multipart_parse_error : public request_parse_error
    
    Thrown when creating a :cpp:class:`multipart`, iterating over parts, or reading from a :cpp:class:`multipart::segment` whenever the content violates the multipart format.
