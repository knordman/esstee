FUNCTION_BLOCK assign_s

VAR_INPUT
	in : INT;
END_VAR

VAR
	out : INT;
END_VAR

out := in;

END_FUNCTION_BLOCK

PROGRAM t

VAR
	a0 : assign_s;
	a1 : assign_s;
	a2 : assign_s;
	a3 : assign_s;
END_VAR

a1(10);
a2(20);


END_PROGRAM

PROGRAM t2

VAR
	a1 : assign_s;
END_VAR

;

END_PROGRAM
