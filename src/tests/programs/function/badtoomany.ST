FUNCTION my_func : INT

VAR
	not_in : INT;
END_VAR

VAR_INPUT
	in1 : INT;
	in2 : INT;
END_VAR

;

END_FUNCTION

PROGRAM t

VAR
	a : INT;
	b : INT;
	c : INT;
	d : INT;
END_VAR

c := my_func(1, 2, 3);

END_PROGRAM