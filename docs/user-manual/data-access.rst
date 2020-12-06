.. _data_access:

Access to data
==============

Once application successfully parses input JSON string,
LwJSON creates set of tokens in hierarchical order with tree for children tokens.

To simplify data extraction and to quickly find/access-to given object and token,
LwJSON implements simple *find* algorithm based on path formatting.

Traverse object
***************

Valid JSON input starts with *object* (``{``) or *array* (``[``). Anything else is invalid JSON object.
When :cpp:func:`lwjson_parse` successfully processes input data, application may access JSON tokens
with simple loop. Every token is part of linked list and has *tree* organization for children objects.

.. note ::
    Children objects are only available for *object* and *array* types

To traverse through all elements, application must first get top object.
It can then loop through all in linked list until it reaches zero.

If the token type is *object* or *array*, application must check children nodes for more token data.

.. literalinclude:: ../../examples/example_traverse.c
    :language: c
    :linenos:
    :caption: Traverse through all tokens

.. tip ::
    Check :cpp:func:`lwjson_print_json` to print data on stream output

Find token in JSON tree
***********************

Instead of manually traversing through all tokens, LwJSON implements simple search algorithm
to quickly get token from application standpoint.

Let's consider ``{"foo1":"bar1","foo2":{"foo3":"bar2"},"a":[{"c":"d"},{"c":"e","f":"g"}]}`` as input JSON string.

* ``foo1`` is string with value ``bar1``
* ``foo2`` is object with sub tokens

  * ``foo3`` is string with value ``bar2`` and is part of ``foo2``
* ``a`` is array of ``2`` objects

  * object 1

    * ``c`` is key in first object, with string value ``d``
  * object 2

    * ``c`` is key with string value ``e``
    * ``f`` is key with string value ``g``

To find the value of key ``foo3``, application must first know where it is located
and then traverse through object to find key by key and enter to object.

To overcome this problem, LwJSON implements *find* functionality to find token.
For search operation, path is separated by comma, thus to get ``foo3`` token,
we need to search for path ``foo2.foo3``.

.. tip::
    Application shall use :cpp:func:`lwjson_find` to get the token

When JSON contains arrays (these do not have keys), special character ``#`` may be used,
indicating *any* element in array to be checked until first match is found.

* ``a.#.c`` will return first token matching path, the one with string value ``d`` in first object
* ``a.#.f`` will return first token matching path, the one with string value ``g`` in second object

.. toctree::
    :maxdepth: 2