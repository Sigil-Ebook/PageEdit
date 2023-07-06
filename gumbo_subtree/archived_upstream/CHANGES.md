Gumbo 0.9.3 (2015-02-17)

* Bugfix for &AElig; entities (rgrove)
* Fix CDATA handling; CDATA sections now generate a GUMBO_NODE_CDATA node rather
than plain text.
* Fix get_title example to handle whitespace nodes (gsnedders)
* Visual Studio compilation fixes (fishioon)
* Take the namespace into account when determining whether a node matches a
certain tag (aroben)
* Replace the varargs tag functions with a tagset bytevector, for a 20-30%
speedup in overall parse time (kevinhendricks, vmg)
* Add MacOS X support to Travis CI, and fix the deployment/DLL issues this
uncovered (nostrademons, kevinhendricks, vmg)

Gumbo 0.9.2 (2014-09-21)

* Performance improvements: Ragel-based char ref decoder and DFA-based UTF8
* decoder, totaling speedups of up to 300%.
* Added benchmarking program and some sample data.
* Fixed a compiler error under Visual Studio.
* Fix an error in the ctypes bindings that could lead to memory corruption in
* the Python bindings.
* Fix duplicate attributes when parsing <isindex> tags.
* Don't leave semicolons behind when consuming entity references (rgrove)
* Internally rename some functions in preparation for an amalgamation file
(jdeng)
* Add proper cflags for gyp builds (skabbes)

Gumbo 0.9.1 (2014-08-07)

* First version listed on PyPi.
* Autotools files excluded from GitHub and generated via autogen.sh. (endgame)
* Numerous compiler warnings fixed. (bnoordhuis, craigbarnes)
* Google security audit passed.
* Gyp support (tfarina)
* Naming convention for structs changed to avoid C reserved words.
* Fix several integer and buffer overflows (Maxime2)
* Some Visual Studio compiler support (bugparty)
* Python3 compatibility for the ctypes bindings.

Gumbo 0.9.0 (2013-08-13)

* Initial release open-sourced by Google.
