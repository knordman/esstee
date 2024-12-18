# esstee

esstee is an interpreter written in C (GNU C99), currently under work, for the
Structured Text (ST) programming language. Structured Text is one of the five
languages defined in the IEC-61131-3 standard, and used a lot in the automation
industry.

The aim of esstee is to provide a, "free as in freedom", platform for building
better testing and debug tools for ST applications.

esstee is licensed under GPLv3. Besides Bison and Flex, esstee has no external
dependencies besides the header libraries included in the source. Compiling has
so far only been tested under Debian, but should work without problems in any
Linux distro.

## Quick Start

The first milestone of esstee is to provide a library that allows ST syntax to
be parsed into a representation which can be stepped/run. The public interface
to the library is defined in `src/esstee/esstee.h`.

Currently, the parsing step of the library is complete (Flex and Bison grammar).
In addition there is support for types (elementary, derived and complex), all
statements and expressions, however it is not yet properly tested (automatic
integration tests not ready), and should be considered as **under work**.

A simple test program using the library interface (and building all the current
code) can be built by;

```
make build/program-tester
```

The program parses ST syntax from a file, then executes possible "pre" queries
on the runnable representation, runs N cycles and finally executes possible
"post" queries. A query is either a variable reference or an assignment. The
program takes the following options:

- **file** the file to parse
- **program** the `PROGRAM` in the file that is to be run (a file may contain multiple programs)
- **pre-run-queries** the queries to run right after parsing
- **run-cycles** number of cycles to run (default 1)
- **post-run-queries** the queries to run after the cycles have been run

In pre and post queries, access the program variables by prefixing with `[name-of-program]`.

An example:

```
./build/program-tester --file="src/tests/programs/example.ST" --program="testprgm" --pre-run-queries="[testprgm].a:=10*5+1" --post-run-queries="[testprgm].a"

```
