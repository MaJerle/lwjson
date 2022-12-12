# Lightweight JSON text parser

Library provides generic JSON text parser, that is optimized for embedded systems.
Supports `streaming` parsing or classic parsing with full JSON data available in one big linear memory.
First one being optimized for ultra small microcontrollers, second one being ready for PC applications - or simply when several kB of RAM memory is available at any given point of time

<h3>Read first: <a href="http://docs.majerle.eu/projects/lwjson/">Documentation</a></h3>

## Features

* Written in ANSI C99, compatible with ``size_t`` for size data types
* RFC 4627 and RFC 8259 compliant
* Based on static token allocation with optional application dynamic pre-allocation
* No recursion during parse operation
* Re-entrant functions
* Zero-copy, no ``malloc`` or ``free`` functions used
* Supports streaming parsing as secondary option
* Optional support for inline comments with `/* comment... */` syntax between any *blank* region of input string
* Advanced find algorithm for tokens
* Test coverage is available
* User friendly MIT license

## Contribute

Fresh contributions are always welcome. Simple instructions to proceed::

1. Fork Github repository
2. Respect [C style & coding rules](https://github.com/MaJerle/c-code-style) used by the library
3. Create a pull request to develop branch with new features or bug fixes

Alternatively you may:

1. Report a bug
2. Ask for a feature request
