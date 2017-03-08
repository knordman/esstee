FUNCTION_BLOCK fb_for

VAR
	itr : INT;
	out : ARRAY[1..5] of INT;
	out2 : ARRAY[1..5] of INT;
END_VAR

for itr := 1 to 5 do
    out[itr] := itr;
    out2[itr] := itr * 2;
end_for;

END_FUNCTION_BLOCK

PROGRAM t_for

VAR
	fb : fb_for;
END_VAR

fb();

END_PROGRAM


FUNCTION_BLOCK fb_while

VAR
	itr : INT := 0;
	control : INT := 0;
	control_two : INT := 0;
	itr_two : INT := 0;
END_VAR

while True do
      itr := itr + 1;

      if itr > 5 then
      	 exit;
      end_if;

      control := control + 1;

end_while;

while False do
      control_two := 100;
end_while;

while itr_two < 4 do
      itr_two := itr_two + 1;
end_while;

END_FUNCTION_BLOCK

PROGRAM t_while

VAR
	fb : fb_while;
END_VAR

fb();

END_PROGRAM


FUNCTION_BLOCK fb_repeat

VAR
	itr : INT := 0;
	control : INT := 0;
	control_two : INT := 0;
	itr_two : INT := 0;
END_VAR

repeat
      itr := itr + 1;

      if itr > 5 then
      	 exit;
      end_if;

      control := control + 1;
until False
end_repeat;

repeat
      control_two := control_two + 1;
until True
end_repeat;

repeat
	itr_two := itr_two + 1;
until itr_two > 4
end_repeat;

END_FUNCTION_BLOCK

PROGRAM t_repeat

VAR
	fb : fb_repeat;
END_VAR

fb();

END_PROGRAM
