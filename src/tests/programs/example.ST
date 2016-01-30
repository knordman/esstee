PROGRAM testprgm

VAR
	myvar : DINT;
	mydint : DINT;
	mybool : BOOL;
END_VAR

myvar := 1000;
mydint := 100;

mybool := (0 <> 0) and ((1 = 1) or (2 = 2));

END_PROGRAM