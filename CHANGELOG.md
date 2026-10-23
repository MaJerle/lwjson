# Changelog

## Develop

- Stream parser: decode escape sequences in strings and keys, including unicode `\uXXXX` to UTF-8
- Stream parser: return error on invalid escape sequence
- Parser: `lwjson_find` matches unescaped key names
- Parser: validate unicode surrogate pairs in strings
- Utils: `lwjson_utils_unescape_string` decodes unicode escape sequences to UTF-8, and returns error on incomplete escape sequence
- Utils: add `lwjson_utils_unescape_seq` and `lwjson_utils_utf8_encode` functions
- Docs: describe string escape handling and stream key name limitations
- Add escape sequence test for parser and stream parser

## 1.10.1

- Fix parsing invalid JSON ending

## 1.10.0

- Fix stream parser: NUL-terminate string buffer before invoking user callback, preventing any stale value
- Refactor the code, add helper functions to reduce code repetition
- Improve the parsing of the comments for JSON with comments

## 1.9.0

- Added the JSON serialized new feature

## 1.8.1

- Fix the platformio library package description

## 1.8.0

- Rework library CMake with removed INTERFACE type
- Improve the calculation with square-multiply algorithm (@SKlimaRA)
- Improve the input boundary checks (@DKubasekRA)

## 1.7.0

- Add clang-tidy
- Add helper functions for sequence check in stream parsing
- Add support to discard invalid JSON stream
- Fixed some invalid JSON parsing in the streaming module

## 1.6.1

- Fix critical issue - missing correct return when waiting for first character. Should be `lwjsonSTREAMWAITFIRSTCHAR`

## 1.6.0

- Split CMakeLists.txt files between library and executable
- Change license year to 2022
- Fix GCC warning for incompatible comparison types
- Update code style with astyle
- Add support for stream parsing - first version
- Add `.clang-format`
- Add `lwjsonSTREAMDONE` return code when streamer well parsed some JSON and reached end of string
- Add option to reset stream state machine

## 1.5.0

- Add string compare feature
- Add string compare with custom length feature

## 1.4.0

- Add support with input string with length specifier
- Add VSCode project for Win32 compilation

## 1.3.0

- Added support for inline `/* */` comments

## v1.2.0

- Added `lwjson_find_ex` function to accept token pointer as starting reference
- Update of the docs for _find_
- Remove unused reset and add free function for future dynamic allocation support

## v1.1.0

- Improved find algorithm to match array index
- Added more test code

## v1.0.2

- Fix wrong parsing of hex in some corner cases
- Add more robust code to handle errorneous JSON input

## v1.0.1

- Added test code
- Fixed bug with improper string parsing

## v1.0.0

- First stable release
- Compliant with RFC 4627 for JSON
- Full features JSON parser
