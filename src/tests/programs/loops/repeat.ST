PROGRAM t

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

END_PROGRAM
