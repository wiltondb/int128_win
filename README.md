int128 library
==============

Minimalistic `int128` library:

 - C99, header-only
 - supports only 64-bit [MSVC](https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B) on Little Endian
 - implements arithmetic operations and bit shifts
 - `divide` operation is based on [Abseil](https://github.com/abseil/abseil-cpp/tree/master/absl/numeric)
 - usage exmaples in [test.c](https://github.com/wiltondb/int128_win/blob/master/test.c)

 Other `int128` libraries that may be of interest: [Boost.Multiprecision](https://github.com/boostorg/multiprecision), [Big-Numbers](https://stackoverflow.com/a/39016672) ([mirror](https://github.com/staticlibs/Big-Numbers/blob/master/Lib/Src/Math/Int128x64.asm)).

License information
-------------------

This project is released under the [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0).