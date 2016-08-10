PROGRAM t

VAR
	from_value : INT;
	to_value : INT;
	itr : INT;
	sum : INT;
END_VAR

for itr := from_value to to_value do
    sum := sum + itr;
end_for;

END_PROGRAM
