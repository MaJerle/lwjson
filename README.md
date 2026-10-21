# Lightweight JSON text parser

LwJSON is a generic JSON parser library optimized for embedded systems.
Supports `streaming` parsing or classic parsing with full JSON data available in one big linear memory.
The first is optimized for ultra-small microcontrollers, while the second is better suited for PC applications, or simply when several kB of RAM are available.

[Open documentation](https://docs.majerle.eu/projects/lwjson/)

## Features

* Written in C (C11), compatible with `stdint.h` data types
* RFC 4627 and RFC 8259 compliant
* Based on static token allocation with optional application dynamic pre-allocation
* No recursion during parse operation
* Re-entrant functions
* Zero-copy, no `malloc` or `free` functions used
* Supports streaming parsing as secondary option
* Optional support for inline comments with `/* comment... */` syntax between any *blank* region of input string
* JSON serializer separate module, with fixed-buffer or callback-driven chunked output
* Optional 64-bit integer serialization
* Stream parser stack-sequence helper macros for matching nested structures
* Distinct integer and real number token types
* Dedicated string escape/unescape utility API
* Advanced find algorithm for tokens
* Test coverage is available
* User friendly MIT license

## Contribute

Fresh contributions are always welcome. Simple instructions to proceed:

1. Fork Github repository
2. Follow [C style & coding rules](https://github.com/MaJerle/c-code-style) and use `clang-format` to format the code
3. Create a pull request to `develop` branch with new features or bug fixes

Alternatively you may:

1. Report a bug
2. Ask for a feature request
