PROGRAM t

VAR
	itr : INT;
	out : ARRAY[1..5] of INT;
END_VAR

for itr := 1 to 5 do
    out[itr] := itr;

    if itr = 2 then
       exit;
    end_if;

end_for;

END_PROGRAM
