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
	a : INT;
END_VAR

a := -(-(1 + 1) * 10);
e := e + a;

return;

END_FUNCTION_BLOCK


PROGRAM testprgm

VAR
	a : INT;
	b : m1; 
END_VAR

b(e:=a);

END_PROGRAM