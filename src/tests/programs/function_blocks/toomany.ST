FUNCTION_BLOCK simple_s

VAR_INPUT
	in1 : INT;
END_VAR

;

END_FUNCTION_BLOCK

(* Too many inputs *)
PROGRAM bad_too_many

VAR
	simple : simple_s;
END_VAR

simple(in1:=1, 10);

END_PROGRAM
