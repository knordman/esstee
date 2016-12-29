FUNCTION_BLOCK simple_s

VAR_INPUT
	in : INT;
END_VAR


VAR_OUTPUT
	out : INT;
END_VAR

VAR
	last : INT;
END_VAR

;

last := out;
out := last + in;


END_FUNCTION_BLOCK

(* Calling variable amount of times *)
PROGRAM t

VAR
	a : INT;
	b : INT;
	itr : INT;
	simple : simple_s;
END_VAR

for itr := 1 to b do
    simple(in:=a);
end_for;

END_PROGRAM

(* Calling without arguments *)
PROGRAM no_args

VAR
	simple : simple_s;
END_VAR

simple();

END_PROGRAM

(* Wrong type for in *)
PROGRAM bad_in_type

VAR
	simple : simple_s;
END_VAR

simple(in:=true and false);

END_PROGRAM
