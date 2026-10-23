.. _stream:

Stream parser
=============

Streaming parser implementation is alternative option versus standard tokenized one, in the sense that:

* There is no need to have full JSON available at one time to have successful parsing
* It can be utilized to parse very large JSON strings on very small systems with limited memory
* It allows users to *take* from the stream only necessary parts and store them to local more system-friendly variable

This type of parser does not utilize use of tokens, rather focuses on the callback function,
where user is in charge to manually understand token structure and get useful data from it.

Stream parser introduces *stack* mechanism instead - to keep the track of depthness during parsing the process.
``3`` different element types are stored on *local stack*:

* Start of object, with ``{`` character
* Start of array, with ``[`` character
* Key from the *object* entry

.. note::
    Stack is nested as long as JSON input stream is nested in the same way

Consider this input string: ``{"k1":"v1","k2":[true, false]}``.
During parsing procedure, at some point of time, these events will occur:

#. Start of *object* detected - *object* pushed to stack

    #. *key* element with name ``k1`` detected and pushed to stack

        #. *string* ``v1`` parsed as *string-value*
    #. *key* element with name ``k1`` popped from stack
    #. *key* element with name ``k2`` detected and pushed to stack

        #. Start of *array* detected - *array* pushed to stack

            #. ``true`` primitive detected
            #. ``false`` primitive detected
        #. End of *array* detected - *array* popped from stack
    #. *key* element with name ``k2`` popped from stack
#. End of *object* detected - *object* popped from stack

Each of these events is reported to user in the callback function.

Strings and keys
****************

Strings and keys are delivered through the ``jsp->data.str`` buffer, with escape sequences already decoded.
``\n`` is delivered as new line character, unicode escape sequences ``\uXXXX`` (including surrogate pairs) are decoded to *UTF-8*.
Invalid escape sequence stops parsing with an error.

When string is longer than ``LWJSON_CFG_STREAM_STRING_MAX_LEN``, it is delivered in multiple chunks,
with multiple callback calls. ``jsp->data.str.is_last`` is set to ``1`` for the last chunk.
Multi-byte *UTF-8* character may be split between two chunks.

.. warning::
    Key name on the stack (``jsp->stack[x].meta.name``) is copied from the string buffer
    once the key is fully parsed. It therefore only holds the **last chunk** of the key,
    and is additionally limited to ``LWJSON_CFG_STREAM_KEY_MAX_LEN`` characters.

    Application must ensure that ``LWJSON_CFG_STREAM_STRING_MAX_LEN`` and ``LWJSON_CFG_STREAM_KEY_MAX_LEN``
    are large enough to hold the longest key in the expected JSON input,
    otherwise key comparison in the callback, based on the stack, will fail.

An example of the stream parsing:

.. literalinclude:: ../../examples/example_stream.c
    :language: c
    :linenos:
    :caption: Parse JSON data as a stream object

Example
*******

For the purpose of example, the following JSON input...

.. literalinclude:: ../examples/stream.json
    :language: json
    :linenos:
    :caption: JSON input for streaming

\... will output the log as:

.. literalinclude:: ../examples/stream_log.txt
    :linenos:
    :caption: JSON development log for the various events

.. toctree::
    :maxdepth: 2