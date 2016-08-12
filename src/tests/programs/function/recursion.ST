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

c := my_func(my_func(4,5),1+1);

d := my_func(a, 10*a);

END_PROGRAM