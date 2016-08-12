FUNCTION my_func : INT

VAR
	not_in : INT;
END_VAR

VAR_INPUT
	in1 : INT;
END_VAR

VAR
	a : INT;
END_VAR

VAR_INPUT
	in2 : INT;
END_VAR

my_func := in1 - in2;

END_FUNCTION

PROGRAM t

VAR
	a : INT;
	b : INT;
	c : INT;
	d : INT;
END_VAR

c := my_func(a,b);

d := my_func(in1:=a,in2:=b);

END_PROGRAM