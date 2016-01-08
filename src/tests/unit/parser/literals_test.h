#include <cxxtest/TestSuite.h>

extern "C" {
#include <parser/parser.h>
}

class CModule_test : public CxxTest::TestSuite
{
public:
	void setup_mock_structs(struct location_t *location, struct parser_t *parser)
	{
		st_parser_init(parser, NULL);
		location->source = NULL;
		location->first_line = 0;
		location->first_column = 0;
		location->last_line = 0;
		location->last_column = 0;
	}

	void print_errors(struct error_t *errors)
	{
		struct error_t *itr;
		for(itr = errors; itr != NULL; itr = itr->next)
		{
			printf("Error: %s\n", itr->description);
		}
	}

	void test_boolean_false()
	{
		struct parser_t parser;
		struct location_t location;

		setup_mock_structs(&(location), &(parser));
		struct literal_t *result = st_new_boolean_literal(0, &(location), &(parser));

		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & BOOLEAN_LITERAL);
		TS_ASSERT((result->literal_class & (~BOOLEAN_LITERAL)) == 0);
		struct integer_literal_t *il = (struct integer_literal_t *)result;
		TS_ASSERT_EQUALS(il->value, 0);
	}

	void test_boolean_true()
	{
		struct parser_t parser;
		struct location_t location;

		setup_mock_structs(&(location), &(parser));
		struct literal_t *result = st_new_boolean_literal(1, &(location), &(parser));

		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & BOOLEAN_LITERAL);
		TS_ASSERT((result->literal_class & (~BOOLEAN_LITERAL)) == 0);

		struct integer_literal_t *il = (struct integer_literal_t *)result;

		TS_ASSERT_EQUALS(il->value, 1);
	}

	void test_integer_decimal()
	{
		struct parser_t parser;
		struct location_t location;
		
		setup_mock_structs(&(location), &(parser));
		struct literal_t *result = st_new_integer_literal(strdup("12345"), &(location), &(parser));

		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & INTEGER_LITERAL);
		TS_ASSERT(result->literal_class & DECIMAL_LITERAL);

		bitflag_t tmp = ~(INTEGER_LITERAL|DECIMAL_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);

		struct integer_literal_t *il = (struct integer_literal_t *)result;
		TS_ASSERT_EQUALS(il->value, 12345)
	}

	void test_integer_decimal_underscore()
	{
		struct parser_t parser;
		struct location_t location;

		struct integer_literal_t *il;
		struct literal_t *result;
		bitflag_t tmp;		

		setup_mock_structs(&(location), &(parser));
		result = st_new_integer_literal(strdup("___12345"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & INTEGER_LITERAL);
		TS_ASSERT(result->literal_class & DECIMAL_LITERAL);
		tmp = ~(INTEGER_LITERAL|DECIMAL_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);
		il = (struct integer_literal_t *)result;
		TS_ASSERT_EQUALS(il->value, 12345)

		setup_mock_structs(&(location), &(parser));
		result = st_new_integer_literal(strdup("22345__"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & INTEGER_LITERAL);
		TS_ASSERT(result->literal_class & DECIMAL_LITERAL);
		tmp = ~(INTEGER_LITERAL|DECIMAL_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);
		il = (struct integer_literal_t *)result;
		TS_ASSERT_EQUALS(il->value, 22345)

		setup_mock_structs(&(location), &(parser));
		result = st_new_integer_literal(strdup("3_2_3__45_"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & INTEGER_LITERAL);
		TS_ASSERT(result->literal_class & DECIMAL_LITERAL);
		tmp = ~(INTEGER_LITERAL|DECIMAL_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);
		il = (struct integer_literal_t *)result;
		TS_ASSERT_EQUALS(il->value, 32345)

		setup_mock_structs(&(location), &(parser));
		result = st_new_integer_literal(strdup("_"), &(location), &(parser));
		TS_ASSERT(result == NULL);		

		setup_mock_structs(&(location), &(parser));
		result = st_new_integer_literal(strdup(""), &(location), &(parser));
		TS_ASSERT(result == NULL);
	}
	
	void test_integer_binary()
	{
		struct parser_t parser;
		struct location_t location;

		struct integer_literal_t *il;
		struct literal_t *result;
		bitflag_t tmp;

		setup_mock_structs(&(location), &(parser));
		result = st_new_integer_literal_binary(strdup("2#1110"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & INTEGER_LITERAL);
		TS_ASSERT(result->literal_class & BINARY_LITERAL);
		tmp = ~(INTEGER_LITERAL|BINARY_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);		
		il = (struct integer_literal_t *)result;
		TS_ASSERT_EQUALS(il->value, 0b1110);

		setup_mock_structs(&(location), &(parser));
		result = st_new_integer_literal_binary(strdup("2#0_00_1__11__00_"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & INTEGER_LITERAL);
		TS_ASSERT(result->literal_class & BINARY_LITERAL);
		tmp = ~(INTEGER_LITERAL|BINARY_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);		
		il = (struct integer_literal_t *)result;
		TS_ASSERT_EQUALS(il->value, 0b00011100);

		setup_mock_structs(&(location), &(parser));
		result = st_new_integer_literal_binary(strdup("2#_"), &(location), &(parser));
		TS_ASSERT(result == NULL);
	}

	void test_integer_octal()
	{
		struct parser_t parser;
		struct location_t location;

		struct integer_literal_t *il;
		struct literal_t *result;
		bitflag_t tmp;

		setup_mock_structs(&(location), &(parser));
		result = st_new_integer_literal_octal(strdup("8#70021"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & INTEGER_LITERAL);
		TS_ASSERT(result->literal_class & OCTAL_LITERAL);
		tmp = ~(INTEGER_LITERAL|OCTAL_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);		
		il = (struct integer_literal_t *)result;
		TS_ASSERT_EQUALS(il->value, 070021);

		setup_mock_structs(&(location), &(parser));
		result = st_new_integer_literal_octal(strdup("8#1_0_7_3__00"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & INTEGER_LITERAL);
		TS_ASSERT(result->literal_class & OCTAL_LITERAL);
		tmp = ~(INTEGER_LITERAL|OCTAL_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);		
		il = (struct integer_literal_t *)result;
		TS_ASSERT_EQUALS(il->value, 0107300);

		setup_mock_structs(&(location), &(parser));
		result = st_new_integer_literal_octal(strdup("8#_"), &(location), &(parser));
		TS_ASSERT(result == NULL);
	}

	void test_integer_hex()
	{
		struct parser_t parser;
		struct location_t location;

		struct integer_literal_t *il;
		struct literal_t *result;
		bitflag_t tmp;

		setup_mock_structs(&(location), &(parser));
		result = st_new_integer_literal_hex(strdup("16#123fedcb00b"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & INTEGER_LITERAL);
		TS_ASSERT(result->literal_class & HEX_LITERAL);
		tmp = ~(INTEGER_LITERAL|HEX_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);		
		il = (struct integer_literal_t *)result;
		TS_ASSERT_EQUALS(il->value, 0x123fedcb00b);

		setup_mock_structs(&(location), &(parser));
		result = st_new_integer_literal_hex(strdup("16#123ABC"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & INTEGER_LITERAL);
		TS_ASSERT(result->literal_class & HEX_LITERAL);
		tmp = ~(INTEGER_LITERAL|HEX_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);		
		il = (struct integer_literal_t *)result;
		TS_ASSERT_EQUALS(il->value, 0x123abc);

		setup_mock_structs(&(location), &(parser));
		result = st_new_integer_literal_hex(strdup("16#__1_2_3A_BC"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & INTEGER_LITERAL);
		TS_ASSERT(result->literal_class & HEX_LITERAL);
		tmp = ~(INTEGER_LITERAL|HEX_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);		
		il = (struct integer_literal_t *)result;
		TS_ASSERT_EQUALS(il->value, 0x123abc);

		setup_mock_structs(&(location), &(parser));	
		result = st_new_integer_literal_hex(strdup("16#_"), &(location), &(parser));
		TS_ASSERT(result == NULL);
	}

	void test_real()
	{
		struct parser_t parser;
		struct location_t location;

		struct real_literal_t *rl;
		struct literal_t *result;
		bitflag_t tmp;

		setup_mock_structs(&(location), &(parser));
		result = st_new_real_literal(strdup("12345.0"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & REAL_LITERAL);
		tmp = ~(REAL_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);		
		rl = (struct real_literal_t *)result;
		TS_ASSERT_EQUALS(rl->value, 12345.0);
	}

	void test_real_exp()
	{
		struct parser_t parser;
		struct location_t location;

		struct real_literal_t *rl;
		struct literal_t *result;
		bitflag_t tmp;

		setup_mock_structs(&(location), &(parser));
		result = st_new_real_literal(strdup("1.2e1"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & REAL_LITERAL);
		tmp = ~(REAL_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);		
		rl = (struct real_literal_t *)result;
		TS_ASSERT_DELTA(rl->value, 12.0, 1e-7);
	}

	void test_string()
	{
		struct parser_t parser;
		struct location_t location;

		struct string_literal_t *sl;
		struct literal_t *result;
		bitflag_t tmp;

		setup_mock_structs(&(location), &(parser));
		result = st_new_double_string_literal(strdup("\"double_string\""), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & DOUBLE_BYTE_STRING_LITERAL);
		tmp = ~(DOUBLE_BYTE_STRING_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);		
		sl = (struct string_literal_t *)result;
		TS_ASSERT(strcmp(sl->value, "double_string") == 0);

		setup_mock_structs(&(location), &(parser));
		result = st_new_single_string_literal(strdup("'single_string'"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & SINGLE_BYTE_STRING_LITERAL);
		tmp = ~(SINGLE_BYTE_STRING_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);		
		sl = (struct string_literal_t *)result;
		TS_ASSERT(strcmp(sl->value, "single_string") == 0);

		setup_mock_structs(&(location), &(parser));
		result = st_new_double_string_literal(strdup("\"\""), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & DOUBLE_BYTE_STRING_LITERAL);
		tmp = ~(DOUBLE_BYTE_STRING_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);		
		sl = (struct string_literal_t *)result;
		TS_ASSERT(strcmp(sl->value, "") == 0);

		setup_mock_structs(&(location), &(parser));
		result = st_new_single_string_literal(strdup("''"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & SINGLE_BYTE_STRING_LITERAL);
		tmp = ~(SINGLE_BYTE_STRING_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);		
		sl = (struct string_literal_t *)result;
		TS_ASSERT(strcmp(sl->value, "") == 0);
	}

	void test_duration()
	{
		struct parser_t parser;
		struct location_t location;

		struct duration_literal_t *dl;
		struct literal_t *result;
		bitflag_t tmp;

		setup_mock_structs(&(location), &(parser));
		result = st_new_duration_literal(strdup("time#1d2h3m4s5ms"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & DURATION_LITERAL);
		tmp = ~(DURATION_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);
		dl = (struct duration_literal_t *)result;
		TS_ASSERT_DELTA(dl->d, 1.0, 1e-4);
		TS_ASSERT_DELTA(dl->h, 2.0, 1e-4);
		TS_ASSERT_DELTA(dl->m, 3.0, 1e-4);
		TS_ASSERT_DELTA(dl->s, 4.0, 1e-4);
		TS_ASSERT_DELTA(dl->ms, 5.0, 1e-4);

		setup_mock_structs(&(location), &(parser));
		result = st_new_duration_literal(strdup("t#4d"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		dl = (struct duration_literal_t *)result;
		TS_ASSERT_DELTA(dl->d, 4.0, 1e-4);
		TS_ASSERT_DELTA(dl->h, 0.0, 1e-4);
		TS_ASSERT_DELTA(dl->m, 0.0, 1e-4);
		TS_ASSERT_DELTA(dl->s, 0.0, 1e-4);
		TS_ASSERT_DELTA(dl->ms, 0.0, 1e-4);

		setup_mock_structs(&(location), &(parser));
		result = st_new_duration_literal(strdup("t#2.2h"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		dl = (struct duration_literal_t *)result;
		TS_ASSERT_DELTA(dl->d, 0.0, 1e-4);
		TS_ASSERT_DELTA(dl->h, 2.2, 1e-4);
		TS_ASSERT_DELTA(dl->m, 0.0, 1e-4);
		TS_ASSERT_DELTA(dl->s, 0.0, 1e-4);
		TS_ASSERT_DELTA(dl->ms, 0.0, 1e-4);

		setup_mock_structs(&(location), &(parser));
		result = st_new_duration_literal(strdup("t#1.2m"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		dl = (struct duration_literal_t *)result;
		TS_ASSERT_DELTA(dl->d, 0.0, 1e-4);
		TS_ASSERT_DELTA(dl->h, 0.0, 1e-4);
		TS_ASSERT_DELTA(dl->m, 1.2, 1e-4);
		TS_ASSERT_DELTA(dl->s, 0.0, 1e-4);
		TS_ASSERT_DELTA(dl->ms, 0.0, 1e-4);

		setup_mock_structs(&(location), &(parser));
		result = st_new_duration_literal(strdup("time#210.1s"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		dl = (struct duration_literal_t *)result;
		TS_ASSERT_DELTA(dl->d, 0.0, 1e-4);
		TS_ASSERT_DELTA(dl->h, 0.0, 1e-4);
		TS_ASSERT_DELTA(dl->m, 0.0, 1e-4);
		TS_ASSERT_DELTA(dl->s, 210.1, 1e-4);
		TS_ASSERT_DELTA(dl->ms, 0.0, 1e-4);

		setup_mock_structs(&(location), &(parser));
		result = st_new_duration_literal(strdup("time#2510ms"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		dl = (struct duration_literal_t *)result;
		if(!result) return;
		TS_ASSERT_DELTA(dl->d, 0.0, 1e-4);
		TS_ASSERT_DELTA(dl->h, 0.0, 1e-4);
		TS_ASSERT_DELTA(dl->m, 0.0, 1e-4);
		TS_ASSERT_DELTA(dl->s, 0.0, 1e-4);
		TS_ASSERT_DELTA(dl->ms, 2510.0, 1e-4);

		setup_mock_structs(&(location), &(parser));
		result = st_new_duration_literal(strdup("time#1d__2h3m_4s_5ms"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & DURATION_LITERAL);
		tmp = ~(DURATION_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);
		dl = (struct duration_literal_t *)result;
		TS_ASSERT_DELTA(dl->d, 1.0, 1e-4);
		TS_ASSERT_DELTA(dl->h, 2.0, 1e-4);
		TS_ASSERT_DELTA(dl->m, 3.0, 1e-4);
		TS_ASSERT_DELTA(dl->s, 4.0, 1e-4);
		TS_ASSERT_DELTA(dl->ms, 5.0, 1e-4);

		setup_mock_structs(&(location), &(parser));
		result = st_new_duration_literal(strdup("time#"), &(location), &(parser));
		TS_ASSERT(result == NULL);

		setup_mock_structs(&(location), &(parser));
		result = st_new_duration_literal(strdup("time#1.2d4.2h"), &(location), &(parser));
		TS_ASSERT(result == NULL);

		setup_mock_structs(&(location), &(parser));
		result = st_new_duration_literal(strdup("time#1.2d4.2h3.1m2.3s4.5ms"), &(location), &(parser));
		TS_ASSERT(result == NULL);

	}

	void test_date()
	{
		struct parser_t parser;
		struct location_t location;

		struct date_literal_t *dl;
		struct literal_t *result;
		bitflag_t tmp;

		setup_mock_structs(&(location), &(parser));
		result = st_new_date_literal(strdup("date#2001-12-01"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & DATE_LITERAL);
		tmp = ~(DATE_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);
		dl = (struct date_literal_t *)result;
		TS_ASSERT_EQUALS(dl->y, 2001);
		TS_ASSERT_EQUALS(dl->m, 12);
		TS_ASSERT_EQUALS(dl->d, 1);

		setup_mock_structs(&(location), &(parser));
		result = st_new_date_literal(strdup("d#2005-01-01"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & DATE_LITERAL);
		tmp = ~(DATE_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);
		dl = (struct date_literal_t *)result;
		TS_ASSERT_EQUALS(dl->y, 2005);
		TS_ASSERT_EQUALS(dl->m, 1);
		TS_ASSERT_EQUALS(dl->d, 1);

		setup_mock_structs(&(location), &(parser));
		result = st_new_date_literal(strdup("d#"), &(location), &(parser));
		TS_ASSERT(result == NULL);

		setup_mock_structs(&(location), &(parser));
		result = st_new_date_literal(strdup("date#"), &(location), &(parser));
		TS_ASSERT(result == NULL);

		setup_mock_structs(&(location), &(parser));
		result = st_new_date_literal(strdup("date#-"), &(location), &(parser));
		TS_ASSERT(result == NULL);

		setup_mock_structs(&(location), &(parser));
		result = st_new_date_literal(strdup("date#0001-13-01"), &(location), &(parser));
		TS_ASSERT(result == NULL);

		setup_mock_structs(&(location), &(parser));
		result = st_new_date_literal(strdup("date#0001-00-01"), &(location), &(parser));
		TS_ASSERT(result == NULL);

		setup_mock_structs(&(location), &(parser));
		result = st_new_date_literal(strdup("date#0000-01-00"), &(location), &(parser));
		TS_ASSERT(result == NULL);

		setup_mock_structs(&(location), &(parser));
		result = st_new_date_literal(strdup("date#0001-01-32"), &(location), &(parser));
		TS_ASSERT(result == NULL);

		setup_mock_structs(&(location), &(parser));
		result = st_new_date_literal(strdup("date#0001-01-00"), &(location), &(parser));
		TS_ASSERT(result == NULL);

		setup_mock_structs(&(location), &(parser));
		result = st_new_date_literal(strdup("date#0001-01-01"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & DATE_LITERAL);
		tmp = ~(DATE_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);
		dl = (struct date_literal_t *)result;
		TS_ASSERT_EQUALS(dl->y, 1);
		TS_ASSERT_EQUALS(dl->m, 1);
		TS_ASSERT_EQUALS(dl->d, 1);
	}

	void test_tod()
	{
		struct parser_t parser;
		struct location_t location;

		struct tod_literal_t *tl;
		struct literal_t *result;
		bitflag_t tmp;

		setup_mock_structs(&(location), &(parser));
		result = st_new_tod_literal(strdup("tod#01:02:03.04"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & TOD_LITERAL);
		tmp = ~(TOD_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);
		tl = (struct tod_literal_t *)result;
		TS_ASSERT_EQUALS(tl->h, 1);
		TS_ASSERT_EQUALS(tl->m, 2);
		TS_ASSERT_EQUALS(tl->s, 3);
		TS_ASSERT_EQUALS(tl->ms, 40);

		setup_mock_structs(&(location), &(parser));
		result = st_new_tod_literal(strdup("tod#00:00:00.00"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & TOD_LITERAL);
		tmp = ~(TOD_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);
		tl = (struct tod_literal_t *)result;
		TS_ASSERT_EQUALS(tl->h, 0);
		TS_ASSERT_EQUALS(tl->m, 0);
		TS_ASSERT_EQUALS(tl->s, 0);
		TS_ASSERT_EQUALS(tl->ms, 0);

		setup_mock_structs(&(location), &(parser));
		result = st_new_tod_literal(strdup("tod#24:00:00.00"), &(location), &(parser));
		TS_ASSERT(result == NULL);

		setup_mock_structs(&(location), &(parser));
		result = st_new_tod_literal(strdup("tod#00:60:00.00"), &(location), &(parser));
		TS_ASSERT(result == NULL);

		setup_mock_structs(&(location), &(parser));
		result = st_new_tod_literal(strdup("tod#00:00:60.00"), &(location), &(parser));
		TS_ASSERT(result == NULL);

		setup_mock_structs(&(location), &(parser));
		result = st_new_tod_literal(strdup("tod#00:00:00.99"), &(location), &(parser));
		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & TOD_LITERAL);
		tmp = ~(TOD_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);
		tl = (struct tod_literal_t *)result;
		TS_ASSERT_EQUALS(tl->h, 0);
		TS_ASSERT_EQUALS(tl->m, 0);
		TS_ASSERT_EQUALS(tl->s, 0);
		TS_ASSERT_EQUALS(tl->ms, 990);		
	}

	void test_date_tod()
	{
		struct parser_t parser;
		struct location_t location;

		struct date_tod_literal_t *dtl;
		struct literal_t *result;
		bitflag_t tmp;

		setup_mock_structs(&(location), &(parser));
		result = st_new_date_tod_literal(strdup("date_and_time#1986-01-21-17:30:13.37"), &(location), &(parser));

		TS_ASSERT(result != NULL);
		if(!result) return;
		TS_ASSERT(result->literal_class & DATE_TOD_LITERAL);
		tmp = ~(DATE_TOD_LITERAL);
		TS_ASSERT((result->literal_class & tmp) == 0);
		dtl = (struct date_tod_literal_t *)result;
		TS_ASSERT_EQUALS(dtl->y, 1986);
		TS_ASSERT_EQUALS(dtl->mon, 1);
		TS_ASSERT_EQUALS(dtl->d, 21);
		TS_ASSERT_EQUALS(dtl->h, 17);
		TS_ASSERT_EQUALS(dtl->min, 30);
		TS_ASSERT_EQUALS(dtl->s, 13);
		TS_ASSERT_EQUALS(dtl->ms, 370);
	}
};

