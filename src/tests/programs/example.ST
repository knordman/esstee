PROGRAM testprgm

TYPE

myenum : (one, two, three) := two;

customdint : DINT;

mysubrange : USINT ( 10 .. 20 ) := 15;

END_TYPE

VAR
	mytypevar : mysubrange;

	myenumone, myenumtwo : myenum;

	mydint : customdint;
END_VAR

(* mydint := mytypevar + 1; *)

mytypevar := 20 + 1;


END_PROGRAM