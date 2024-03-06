# Changelog

## Develop

- Add clang-tidy
- Add helper functions for sequence check in stream parsing

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
- Update of the docs for *find*
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
