FUNCTION my_min : INT

VAR_INPUT
	val1 : INT;
	val2 : INT;
END_VAR

if val1 < val2 then
   my_min := val1;
else
   my_min := val2;
end_if;

END_FUNCTION

PROGRAM t

VAR
	a : INT;
	b : INT;
	c : INT;
	d : INT;
END_VAR

c := my_min(a, b);

d := my_min(b, a);

END_PROGRAM