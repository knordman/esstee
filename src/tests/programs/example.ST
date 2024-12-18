PROGRAM testprgm

VAR
	i: INT;
	a: UINT;
END_VAR

FOR i := 1 TO 20 DO

    IF i > 10 THEN
       a := 99;
    ELSE

        a := 10 - i;

    END_IF;

END_FOR;


END_PROGRAM