
FUNCTION myfunc : INT

VAR_INPUT
	a : INT;
END_VAR

myfunc := a + 1;

END_FUNCTION




FUNCTION_BLOCK m1

VAR_INPUT
	e : SINT;
END_VAR

VAR
	a : SINT;
	b : SINT;
END_VAR

b := 127 / 0;
a := -(-(1 + 1) * 10);
e := e + a;

return;

END_FUNCTION_BLOCK





PROGRAM testprgm

TYPE
	mytype : USINT;

END_TYPE

VAR
	a : INT;	
	b : m1;
END_VAR


a(e:=127);
b(e:=10);

END_PROGRAM