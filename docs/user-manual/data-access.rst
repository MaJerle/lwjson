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

Let's consider following JSON as input:

.. code::

    {
        "name":"John",
        "born": {
            "city": "Munich",
            "year": 1993
        },
        "cars":[
            {
                "brand":"Porsche",
                "year":2018
            },
            {
                "brand":"Range",
                "year":2020,
                "repainted":true
	        }
        ]
    }

There is one *John*, born in *Munich* in ``1993`` and has ``2`` cars, *Porsche* and *Range*.

* ``name`` is string with value ``John``
* ``born`` is object with ``2`` fields
* ``cars`` is array of ``2`` objects

  * object 1

    * ``brand`` is set to *Porsche*
    * ``year`` is set to ``2018``
  * object 2

    * ``brand`` is set to *Range*
    * ``year`` is set to ``2020``
    * ``repainted`` is set to ``true`` as this car was recently repainted

To find the person's name, application must first ``name`` key and print its value.
This can be done by scanning entire object and check which token matches ``name`` keyword.

LwJSON implements *find* functionality to find the token in simple way.
This is done by providing full path to token in JSON tree, separated by *dot* ``.`` character.

To find person name, application would then simply call :cpp:func:`lwjson_find` and pass ``name`` as path parameter.
If token exists, function will return handle of the token where application can print its value.

If application is interested in *city of birth*, it will set path as ``born.city`` and search
algorithm will:

* First search for ``born`` token and check if it is *object*
* It will enter the object and search for ``city`` token and return it on match
* It will return ``NULL`` of object is not found

.. tip::
    Application shall use :cpp:func:`lwjson_find` to get the token based on input path.

When JSON contains arrays (these do not have keys), special character ``#`` may be used,
indicating *any* element in array to be checked until first match is found.

* ``cars.#.brand`` will return first token matching path, the one with string value ``Porsche`` in first object
* ``cars.#.repainted`` will return first token matching path, the one with boolean value ``true`` in second object

In first case, ``brand`` keyword exists already in first object, so find function will get the match immediately.
Because in second case, ``repainted`` only exists in second object, function will return value from second object.

Access array index
******************

It is possible to access specific array index by adding decimal number after hash character, in format ``#[0-9]+``

* ``cars.#0.brand`` will return ``brand`` token from *first* object in array (``index = 0``) with value set to *Porsche*
* ``cars.#1.brand`` will return ``brand`` token from *second* object in array (``index = 1``) with value set to *Range*

To retrieve full object of the array, application may only apply ``#[0.9]+`` in search pattern.

* ``cars.#0`` will return first object token in array
* ``cars.#1`` will return second object token in array
* ``cars.#`` will return error as there is no valid index. Use ``cars`` to retrieve full array

.. warning::
    Passing path in format ``path.to.cars.#`` (hashtag as last element without index number) will always return ``NULL``
    as this is considered invalid path. To retrieve full array, pass path to array ``path.to.cars`` only, without trailling ``#``.

.. toctree::
    :maxdepth: 2
