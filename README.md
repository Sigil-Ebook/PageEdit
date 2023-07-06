Sigil-Gumbo - A pure-C HTML5 parser.
===================================

This is a customized and highly updated version of Google's gumbo html parser built
on top of on the following repository as its initial commit:

    https://github.com/vmg/gumbo-parser/tree/v1.0.0

Gumbo is an Apache Licensed implementation of the [HTML5 parsing algorithm][] implemented
as a pure C99 library with no outside dependencies.  It's designed to serve
as a building block for other tools and libraries such as linters,
validators, templating languages, and refactoring and analysis tools.

This repository has forked from the main repository to include memory improvements, and parser 
simplifications, speed improvements, and interface changes that allow simple editing of the tree
post parsing.

It has also been modified to support the use of xhtml parsing rules (turned on and off by settings)
(see the XMTML5 parsing comments in src/parser.c) and modified to recognize all current html, svg
and mathml tags in the tag enum.

Due to recognizing a larger set of tags, we use our own version of tag_perf.h which is a variation
of a minimal perfect hash function for our much larger set of tags.

You can not simply replace this version of gumbo with the one from google/gumbo-parser without breaking Sigil.

Original Goals & features:

* Fully conformant with the latest HTML spec.
* Robust and resilient to bad input.
* Simple API that can be easily wrapped by other languages.
* Support for source locations and pointers back to the original text.
* Support for fragment parsing.
* Relatively lightweight, with no outside dependencies.
* Passes all [html5lib tests][], including the template tag.
* Tested on over 2.5 billion pages from Google's index.


Installation
============

To build and install the library, use CMake version 3.0 or later

    git clone https://github.com/Sigil-Ebook/sigil-gumbo.git
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ../sigil-gumbo
    make -j4


To run the html5lib tree-construction test suite from the build directory
after building
    
    cd python/gumbo
    python3 ./html5lib_adapter_test.py



If GoogleTest was found during the build you should be able to run
our unit tests from the build directory as follows:

    cd bin
    ./run_tests
    

There are a number of sample programs in the examples/ directory that are
compiled and stored in the build directory in bin automatically.

Basic Usage
===========

Within your program, you need to include "gumbo.h" and then issue a call to
`gumbo_parse`:

```C
#include "gumbo.h"

int main() {
  GumboOutput* output = gumbo_parse("<h1>Hello, World!</h1>");
  // Do stuff with output->root
  gumbo_destroy_output(&kGumboDefaultOptions, output);
}
```


Modifying this Software
=======================

Many of the source files use gperf perfect hash algorithms and are
therefore generated.  This archive includes pre-generated versions of
all needed source files.  But if any of them need to be modified,
then new source files will need to be gnerated:

- To rebuild the set of recognized htmnl5 elements do the following
  from main directory:
    
    python3 gentags.py src/tags.in


- To rebuild the set of svg name related fixups do the following
  from the main directory:

    gperf -m100 src/svg_tags.gperf > src/svg_tags.c


- To rebuild the set of svgt attribute name related fixups do the
  following from the main directory:
    
    gperf -m100 src/svg_attrs.gperf > src/svg_attrs.c

    
- To rebuild the set of foreign attribute name fixups do the
  following from the main directory:

    gperf -m100 -n src/foreign_attrs.gperf > src/foreign_attrs.c 
     
 
    
See the API documentation from the archived google gumbo github site
and sample programs for more details.

Recommended best-practice for Python usage is to use one of the adapters
as an existing API and write your program in terms of those.
(see the python/gumbo directory) for html5lib and gumbo and bs4 adapter code.

The raw CTypes bindings should be considered building
blocks for higher-level libraries and rarely referenced directly.

[Archived Google Gumbo github site]: https://github.com/google/gumbo-parser
[HTML5 parsing algorithm]: http://www.whatwg.org/specs/web-apps/current-work/multipage/#auto-toc-12
[HTML5 spec]: http://www.whatwg.org/specs/web-apps/current-work/multipage/
[html5lib tests]: https://github.com/html5lib/html5lib-tests
[googletest]: https://code.google.com/p/googletest/
