.. _data_access:

Access to data
==============

Once application successfully parses input JSON string,
LwJSON creates set of tokens in hierarchical order with tree for children tokens.

To simplify data extraction and to quickly find/access-to given object and token,
LwJSON implements simple *find* algorithm based on path formatting.

Find token in JSON tree
***********************

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
indicating *any* element in array to be matched until first match is found.

* ``a.#.c`` will return first token matching path, the one with string value ``d`` in first object
* ``a.#.f`` will return first token matching path, the one with string value ``g`` in second object

.. toctree::
    :maxdepth: 2