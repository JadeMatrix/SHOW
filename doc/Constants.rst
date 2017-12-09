=========
Constants
=========

.. cpp:namespace-push:: show

All constants are ``const``-qualified.

Version
=======

The ``version`` sub-namespace contains information about the current SHOW version.  It has the following members:

.. cpp:namespace-push:: version

.. cpp:var:: std::string name
    
    The proper name of SHOW as it should appear referenced in headers, log messages, etc.

.. cpp:var:: int major
    
    The major SHOW version (``X.0.0``)

.. cpp:var:: int minor
    
    The minor SHOW version (``0.X.0``)

.. cpp:var:: int revision
    
    The SHOW version revision (``0.0.X``)

.. cpp:var:: std::string string
    
    A string representing the major, minor, and revision version numbers
