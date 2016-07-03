(*
FUNCTION myfunc : INT

VAR_INPUT
	a : INT;
END_VAR

myfunc := a + 1;

END_FUNCTION




FUNCTION_BLOCK m1

VAR_INPUT
	e : SINT;
END_VAR

VAR
	a : SINT;
	b : SINT;
END_VAR

b := 127 / 1;
a := -(-(1 + 1) * 10);
e := e + a;

return;

END_FUNCTION_BLOCK



*)

PROGRAM testprgm

(*
TYPE
	mytype : USINT;

END_TYPE
*)

VAR
	i : INT;
	a : UINT;
(*
	myvar AT %MB0 : USINT;
	m2 : DINT;

	b1 AT %MX0.0 : BOOL := true;
	b2 : BOOL;

*)

END_VAR

(*
i := 5;

a := i - 10;
*)


FOR i := 1 to 20 DO

    if i > 10 then
       a := 99;
    else

        a := 10-i;

    end_if;

END_FOR;


END_PROGRAM