LwJSON |version| documentation
==============================

Welcome to the documentation for version |version|.

LwJSON is a generic JSON parser library optimized for embedded systems.

.. image:: static/images/logo.svg
    :align: center

.. rst-class:: center
.. rst-class:: index_links

	:ref:`download_library` :ref:`getting_started` `Open Github <https://github.com/MaJerle/lwjson>`_ `Donate <https://paypal.me/tilz0R>`_

Features
^^^^^^^^

* Written in ANSI C99, compatible with ``size_t`` for size data types
* RFC 4627 compliant
* Based on static token allocation with optional application dynamic pre-allocation
* No recursion during parse operation
* Re-entrant functions
* Zero-copy, no ``malloc`` or ``free`` functions used
* Advanced find algorithm for tokens
* Testscoverage is available
* User friendly MIT license

Requirements
^^^^^^^^^^^^

* C compiler

Contribute
^^^^^^^^^^

Fresh contributions are always welcome. Simple instructions to proceed:

#. Fork Github repository
#. Respect `C style & coding rules <https://github.com/MaJerle/c-code-style>`_ used by the library
#. Create a pull request to ``develop`` branch with new features or bug fixes

Alternatively you may:

#. Report a bug
#. Ask for a feature request

Example code
^^^^^^^^^^^^

Minimalistic example code to read and write data to buffer

.. literalinclude:: examples_src/example_index.c
    :language: c
    :linenos:
    :caption: Example code

License
^^^^^^^

.. literalinclude:: ../LICENSE

Table of contents
^^^^^^^^^^^^^^^^^

.. toctree::
    :maxdepth: 2

    self
    get-started/index
    user-manual/index
    api-reference/index
