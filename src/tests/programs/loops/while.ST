PROGRAM t

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

END_PROGRAM
