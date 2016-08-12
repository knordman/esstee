FUNCTION my_func : INT

VAR_INPUT
	in1 : INT;
	in2 : INT;
END_VAR

my_func := in1 + in2;

END_FUNCTION

PROGRAM t

VAR
	a : INT;
	b : INT;
	c : INT;
	d : INT;
END_VAR

c := my_func(1+1, my_func(4*10, 5));

d := my_func(a, a*10);

END_PROGRAM