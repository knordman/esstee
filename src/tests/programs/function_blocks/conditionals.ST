FUNCTION_BLOCK fb_if

VAR
	i1 : DINT;
	i2 : DINT;
END_VAR

if i1 = 1 then
   i2 := 10;
elsif i1 = 2 then
   i2 := 20;
elsif i1 = 3 then
   i2 := 30;
else
   i2 := 40;
end_if;

END_FUNCTION_BLOCK


PROGRAM t_if

VAR
	fb : fb_if;
END_VAR

fb();

END_PROGRAM


FUNCTION_BLOCK fb_case

VAR
	i1 : DINT;
	i2 : DINT;
END_VAR

case i1 of

1: i2 := 10;

2: i2 := 20;

3: i2 := 30;

else i2 := 40;

end_case;

END_FUNCTION_BLOCK


PROGRAM t_case

VAR
	fb : fb_case;
END_VAR

fb();

END_PROGRAM
