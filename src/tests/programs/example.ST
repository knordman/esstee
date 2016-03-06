FUNCTION myfunc : INT

VAR_INPUT
	a : INT;
END_VAR

myfunc := a + 1;

END_FUNCTION

FUNCTION_BLOCK m1

VAR_INPUT
	e : INT;
END_VAR

VAR

END_VAR

e := 1;
e := 2;

return;

END_FUNCTION_BLOCK


PROGRAM testprgm

VAR
	a : BOOL;


END_VAR


a := myfunc(a:=10) = 12;


END_PROGRAM