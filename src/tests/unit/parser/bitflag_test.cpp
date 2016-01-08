/*
Copyright (C) 2015 Kristian Nordman

This file is part of esstee. 

esstee is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

esstee is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with esstee.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <util/bitflag.h>

#include <catch.hpp>

#define B0 (1 << 0)
#define B1 (1 << 1)
#define B2 (1 << 2)


TEST_CASE("Setting flags", "ST_SET_FLAGS") {

    st_bitflag_t flags = 0;
    
    SECTION("Set flags, order one") {

	ST_SET_FLAGS(flags, B0);
	REQUIRE( flags == 0b1 );

	ST_SET_FLAGS(flags, B1);
	REQUIRE( flags == 0b11 );

	ST_SET_FLAGS(flags, B2);
	REQUIRE( flags == 0b111 );
    }

    SECTION("Set flags, opposite order") {

	ST_SET_FLAGS(flags, B2);
	REQUIRE( flags == 0b100 );

	ST_SET_FLAGS(flags, B1);
	REQUIRE( flags == 0b110 );

	ST_SET_FLAGS(flags, B0);
	REQUIRE( flags == 0b111 );
    }

    SECTION("Set flags, ored") {

	ST_SET_FLAGS(flags, B0|B2);

	REQUIRE( flags == 0b101 );
    }
}

TEST_CASE("Clearing flags", "ST_CLEAR_FLAGS") {

    st_bitflag_t flags = 0b111;
    
    SECTION("Clear flags one at a time") {

	ST_CLEAR_FLAGS(flags, B0);
	REQUIRE( flags == 0b110 );

	ST_CLEAR_FLAGS(flags, B2);
	REQUIRE( flags == 0b010 );

	ST_CLEAR_FLAGS(flags, B1);
	REQUIRE( flags == 0b000 );
    }

    SECTION("Clear flags, ored") {

	ST_CLEAR_FLAGS(flags, B0|B2);

	REQUIRE( flags == 0b010 );
    }
}

TEST_CASE("Is flag set", "ST_FLAG_IS_SET") {

    st_bitflag_t flags = 0b101;
    
    SECTION("Check flags one at a time") {

	bool b0_is_set = ST_FLAG_IS_SET(flags, B0);
	bool b1_is_set = ST_FLAG_IS_SET(flags, B1);
	bool b2_is_set = ST_FLAG_IS_SET(flags, B2);

	REQUIRE( b0_is_set == true );
	REQUIRE( b1_is_set == false );
	REQUIRE( b2_is_set == true );
    }

    SECTION("Ored flag checks") {

	bool either_b0_or_b2_is_set = ST_FLAG_IS_SET(flags, B0|B2);

	REQUIRE( either_b0_or_b2_is_set == true );
    }

    SECTION("Multiple and conditions") {

	bool b2_and_not_b1_is_set = ST_FLAG_IS_SET(flags, B2) && !ST_FLAG_IS_SET(flags, B1);

	REQUIRE( b2_and_not_b1_is_set == true );
    }
}
