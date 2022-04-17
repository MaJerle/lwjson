.. _how_it_works:

How it works
============

LwJSON fully complies with *RFC 4627* memo and supports ``2`` types of parsing:

* Parsing with full data available as single linear memory (primary option)
* Stream parsing with partial available bytes at any given point of time - advanced state machine

When full data are available, standard parsing is used with tokens,
that contain references to start/stop indexes of the strings and other primitives and provide
full device tree - sort of custom hash-map value.

When JSON is successfully parsed, there are several tokens used, one for each JSON data type.
Each token consists of:

* Token type
* Token parameter name (*key*) and its length
* Token value or pointer to first child (in case of *object* or *array* types)

As an example, JSON text ``{"mykey":"myvalue"}`` will be parsed into ``2`` tokens:

* First token is the opening bracket and has type *object* as it holds children tokens
* Second token has name ``mykey``, its type is *string* with value set as ``myvalue``

.. warning::
    When JSON input string is parsed, create tokens use input string as a reference.
    This means that until JSON parsed tokens are being used, original text must stay as-is.
    Any modification of source JSON input may destroy references from the token tree and hence generate wrong output for the user

.. tip::
    See :ref:`stream` for implementation of streaming parser where full data do not need to be available at any given time.

.. toctree::
    :maxdepth: 2