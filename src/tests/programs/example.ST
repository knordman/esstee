FUNCTION_BLOCK m1

VAR_INPUT
	e : INT;
END_VAR

VAR

END_VAR

;

END_FUNCTION_BLOCK


PROGRAM testprgm

VAR
	a : INT := 0;
	b : INT := 0;
	c : INT;
	d : INT;

	ma : m1;
	mb : m1;
END_VAR

repeat
	a := a + 1;
	b := b - 1;
until a > 3
end_repeat;


END_PROGRAM