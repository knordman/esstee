PROGRAM t_if

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

END_PROGRAM

PROGRAM t_if_simple

VAR
	i1 : DINT;
	i2 : DINT := 20;
END_VAR

if i1 = 1 then
   i2 := 10;
end_if;

END_PROGRAM


PROGRAM t_case

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

END_PROGRAM

