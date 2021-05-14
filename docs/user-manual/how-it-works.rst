.. _how_it_works:

How it works
============

LwJSON fully complies with *RFC 4627* memo.

LwJSON accepts input string formatted as JSON as per *RFC 4627*.
Library parses each character and creates list of tokens that are
understood by C language for easier further processing.

When JSON is successfully parsed, there are several tokens used, one for each JSON data type.
Each token consists of:

* Token type
* Token parameter name (*key*) and its length
* Token value or pointer to first child (in case of *object* or *array* types)

As an example, JSON text ``{"mykey":"myvalue"}`` will be parsed into ``2`` tokens:

* First token is the opening bracket and has type *object* as it holds children tokens
* Second token has name ``mykey``, its type is *string* and value is ``myvalue``

.. warning::
    When JSON input string is parsed, create tokens use input string as a reference.
    This means that until JSON parsed tokens are being used, original text must stay as-is.

.. toctree::
    :maxdepth: 2