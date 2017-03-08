FUNCTION return_value : INT

VAR_INPUT
	val1 : INT;
	val2 : INT;
END_VAR

return_value := val1 + val2;

END_FUNCTION


FUNCTION_BLOCK test_fb

VAR_INPUT
	a : INT;
	b : INT;
END_VAR

VAR
	c : INT;
END_VAR

c := return_value(a, b);

END_FUNCTION_BLOCK


PROGRAM t

VAR
	fb : test_fb;
END_VAR

fb(1, 2);

END_PROGRAM