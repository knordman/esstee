FUNCTION_BLOCK test_fb

VAR_INPUT
	ai : INT;
	bi : INT;
END_VAR

VAR
	a : ARRAY[1..5] of INT;
	b : ARRAY[1..5] of INT;
END_VAR

a[ai] := ai*100;
b[bi] := bi*100;

END_FUNCTION_BLOCK

PROGRAM t

VAR
	t1 : test_fb;
	t2 : test_fb; 
END_VAR

t1(ai:=1, bi:=2);
t2(ai:=5, bi:=3);

END_PROGRAM


FUNCTION_BLOCK test_fb_nest

VAR
	t1 : test_fb;
END_VAR

t1(ai:=1, bi:=2);

END_FUNCTION_BLOCK


PROGRAM tnest

VAR
	t : test_fb_nest;
END_VAR

t();

END_PROGRAM
