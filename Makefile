# ------------------------------------------------------------------------------
#  Configuration options
# ------------------------------------------------------------------------------

CXX := 				g++
CC := 				gcc

CFLAGS :=			-Isrc -Ilib -ggdb3 -Wall -Werror -std=gnu99
CXXFLAGS :=			-Isrc -Ilib -ggdb3 -Wall -Werror -std=c++11

LDFLAGS :=			-lm

# ------------------------------------------------------------------------------
#  Target specification
# ------------------------------------------------------------------------------

OBJECTS := 		build/util/errorcontext.o \
			build/util/config.o \
			build/util/namedrefpool.o \
			build/parser/ptypes.o \
			build/parser/ppous.o \
			build/parser/parser.o \
			build/parser/pvalues.o \
			build/parser/pliterals.o \
			build/parser/pvariables.o \
			build/parser/pdirectmemory.o \
			build/parser/pexpressions.o \
			build/parser/pstatements.o \
			build/parser/pshared.o \
			build/parser/pquery.o \
			build/elements/shared.o \
			build/elements/types.o \
			build/elements/values.o \
			build/elements/pous.o \
			build/elements/directmemory.o \
			build/elements/statements.o \
			build/elements/expressions.o \
			build/elements/query.o \
			build/api/esstee.o \
			build/api/elements.o \
			build/linker/lpous.o \
			build/linker/lvariables.o \
			build/linker/ltypes.o \
			build/linker/lstatements.o \
			build/linker/lexpressions.o \
			build/linker/linker.o \
			build/linker/lshared.o \
			build/linker/lliterals.o \
			build/rt/systime.o \
			build/rt/runtime.o \
			build/rt/cursor.o \
			build/parser/bison.tab.o \
			build/parser/flex.o

build/program-tester :	build/tests/programs/main.o \
			$(OBJECTS)
	$(LINKCC)

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
