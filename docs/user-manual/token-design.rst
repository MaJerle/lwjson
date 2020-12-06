.. _token_design:

Token design
============

Every element of LwJSON is a token.
There are different set of token types:

* *Object*: Type that has nested key-value pairs, eg ``{"key":{"sub-key":"value"}}``
* *Array*: Type that holds nested values, eg ``{"key":[1,2,3,4,5]}``
* *String*: Regular string, quoted sequence of characters, eg ``{"key":"my_string"}``
* *Number*: Integer or real number, eg ``{"intnum":123,"realnum":4.3}``
* *Boolean true*: Boolean type true, eg ``{"key":true}``
* *Boolean false*: Boolean type false, eg ``{"key":false}``
* *Null*: Null indicator, eg ``{"key":null}``

When parsed, input string is not copied to token, every token uses input string as
a reference and points to the beginning of strings/values.
This is valid for all string data types and for parameter names.

.. note ::
    Input string is not modified therefore all strings contain additional
    parameter with string length.

.. toctree::
    :maxdepth: 2