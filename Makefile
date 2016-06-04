# ------------------------------------------------------------------------------
#  Configuration options
# ------------------------------------------------------------------------------

CXX := 				g++
CC := 				gcc

CFLAGS :=			-Isrc -Ilib -ggdb3 -Wall -Werror -std=gnu99 -fPIC
CXXFLAGS :=			-Isrc -Ilib -ggdb3 -Wall -Werror -std=c++11

LDFLAGS :=			-lm

# ------------------------------------------------------------------------------
#  Target specification
# ------------------------------------------------------------------------------

OBJECTS := 		build/util/issue_context.o \
			build/util/config.o \
			build/util/named_ref_pool.o \
			build/parser/parser.o \
			build/parser/parray.o \
			build/parser/pcases.o \
			build/parser/pconditionals.o \
			build/parser/pdirectmemory.o \
			build/parser/penums.o \
			build/parser/pheader.o \
			build/parser/pinvokeparameters.o \
			build/parser/pliterals.o \
			build/parser/ppous.o \
			build/parser/pqualified_identifier.o \
			build/parser/pvariables.o \
			build/parser/ptypes.o \
			build/parser/pexpressions.o \
			build/parser/pstatements.o \
			build/parser/pquery.o \
			build/elements/array.o \
			build/elements/date_time.o \
			build/elements/derived.o \
			build/elements/directmemory.o \
			build/elements/enums.o \
			build/elements/integers.o \
			build/elements/invoke_parameters.o \
			build/elements/qualified_identifier.o \
			build/elements/queries.o \
			build/elements/reals.o \
			build/elements/strings.o \
			build/elements/struct.o \
			build/elements/subrange.o \
			build/elements/user_programs.o \
			build/elements/user_functions.o \
			build/elements/user_function_blocks.o \
			build/elements/values.o \
			build/elements/types.o \
			build/elements/variable.o \
			build/api/esstee.o \
			build/api/elements.o \
			build/linker/linker.o \
			build/rt/systime.o \
			build/rt/cursor.o \
			build/parser/bison.tab.o \
			build/parser/flex.o

build/lib/libesstee.a : $(OBJECTS)
	@mkdir -pv $(dir $@)
	ar rcs $@ $^

build/lib/libesstee.so : SO=.1
build/lib/libesstee.so : REAL=.1.0.1
build/lib/libesstee.so : $(OBJECTS)
	@mkdir -pv $(dir $@)
	gcc -shared -Wl,-soname,$(notdir $@)$(SO) -o $@$(REAL) $^
	ln -fs $(abspath $@)$(REAL) $(abspath $@)$(SO)
	ln -fs $(abspath $@)$(SO) $(abspath $@)

build/program-tester :	build/tests/programs/main.o build/lib/libesstee.a
	gcc $(LDFLAGS) $^ -o $@
#	gcc $< $(LDFLAGS) -Lbuild/lib -lesstee -o $@

build/tester : 		build/tests/temp/main.o \
			$(OBJECTS)

# ------------------------------------------------------------------------------
#  Unit test specification
# ------------------------------------------------------------------------------

tests : build/bitflag_test

build/bitflag_test :			build/tests/unit/parser/bitflag_test.o \
					build/tests/unit/main.o
	$(LINKCXX)

# ...

# ------------------------------------------------------------------------------
#  General targets
# ------------------------------------------------------------------------------

docs :	
	doxygen doc/Doxyfile
clean :
	rm -rf build

.PHONY : clean all tests docs

# -----------------------------------------------------------------------------
#  Implicit rules
# -----------------------------------------------------------------------------

LINKCC = 	gcc $^ $(LDFLAGS) -o $@
LINKCXX = 	g++ $^ $(LDFLAGS) -o $@

src/%.tab.c : src/%.y
	bison -d -v -t $^ -o $@

src/%.c : src/%.l
	flex --header-file=$(dir $@)flex.h -o $@ $^

build/%.o : src/%.cpp
	@mkdir -pv $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/%.o : src/%.c src/%.h
	@mkdir -pv $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o : src/%.c
	@mkdir -pv $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/%.E : src/%.c Makefile
	@mkdir -pv $(dir $@)
	$(CC) $(CFLAGS) -E $< -o $@

build/%.E : src/%.cpp Makefile
	@mkdir -pv $(dir $@)
	$(CXX) $(CFLAGS) -E $< -o $@
