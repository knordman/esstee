PROGRAM t

VAR
	itr : INT;
	out : ARRAY[1..5] of INT;
	out2 : ARRAY[1..5] of INT;
END_VAR

for itr := 1 to 5 do
    out[itr] := itr;
    out2[itr] := itr * 2;
end_for;


END_PROGRAM
