# Running and writing tests

The simple `runtests.sh` script is designed for running integration
tests, i.e. tests for the complete interpreter chain. The script reads
test definitions from stdin and executes them using the
`program-tester` binary. To run tests, first build the `program-tester`
by issuing (in the root project folder)

```
make program-tester
```

Test defintions can be grouped together in files (like the files found
in the folder of this file), and then executed by

```
cat tests | ./runtests.sh
```

The success or failure of the tests are written to stdout, while any
output from the tests are written to the file `error.output`. Note
that, each test rewrites `error.output`, so it will only contain the
output of the last run test.

## Test definition structure

A test definition consists of six fields separated by '!'. Order and
contents of the fields:
1. The path to the .ST file which is to be loaded and run
2. The program in the .ST file that is to be run (the file may contain multiple programs)
3. The expected output from the `program-tester` binary (1 for error and 0 for success)
4. The queries that are to be executed in the interpreter before running a cycle of the program
5. The queries that are to be executed in the interpreter after the cycle has finished
6. The excepted output from `program-tester`, this will be set by the post-cycle queries

### Queries

Pre- and post-cycle queries are given in ST syntax. Output from
pre-cycle queries is ignored, while the output from the post cycle
queries is checked for equivalence with the expected output. A query
is either a reference to a variable, or an assignment
statement. Multiple queries are separated by ';'. When referring to
variables of a program, enclose the program name in '[]'. The
following would print the value of the variable 'my_var' inside the
function block 'my_fb' part of 'my_prgm', and also set the value of
the global variable 'global_var'. Note that the last query has no
trailing ";".

```
[my_prgm].my_fb.my_var;global_var:=1+2*3
```

## Options to runtests.sh

The following options are possible to pass to `runtests.sh`:

```
cat tests | ./runtests.sh [filter] [gdb / bison]
```

The filter option is an awk regex which can limit which tests are run,
i.e. for the test to be run its defintion has to match the filter
regex. 

If a filter is passed, together with "gdb", the `gdb` commands needed
to run the first matched test will be written to
`test.gdb.commands`. This includes setting the binary file and
arguments to the program. To load the test in `gdb`, open up `gdb` and
execute:

```
source test.gdb.commands
```

If a filter is passed, together with "bison", the output in
`error.output` will include bison debug information.
