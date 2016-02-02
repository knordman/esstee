PROGRAM testprgm

TYPE

myenum : (one, two, three) := two;

customdint : DINT;

END_TYPE

VAR
	mytypevar : BOOL;

	myenumone, myenumtwo : myenum;

	mydint : customdint;
END_VAR

mytypevar := (myenumone = two);

END_PROGRAM