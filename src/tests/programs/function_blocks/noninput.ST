FUNCTION_BLOCK simple_s

VAR_INPUT
	in : INT;
END_VAR

;

END_FUNCTION_BLOCK

(* Assigning non-input variables *)
PROGRAM bad_non_input

VAR
	simple : simple_s;
END_VAR

simple(last:=1);

END_PROGRAM
