PROGRAM testprgm

TYPE

myenum : (one, two, three) := two;

customdint : DINT := 5;


mysubrange : USINT ( 10 .. 20 ) := 15;

myarray : ARRAY [1..3,1..2,1..2] OF SINT :=
[
 [
  [1,2],[3,4]
 ],
 [
  [5,6],[7,8]
 ],
 [
  [9,10],[11,12]
 ]
];


mstr :
STRUCT
	one_int : INT;
	another_int : INT := 200;
END_STRUCT;

myotherstruct :	
STRUCT
	container : mystruct;
	myvar : INT;
END_STRUCT;



mystruct :
STRUCT
	firstmember : customdint;
	secondmember : mstr;
END_STRUCT;


END_TYPE


VAR
	mytypevar : BOOL;

	myenumone, myenumtwo : myenum;


	mysint : SINT := 1;

	mydint : BOOL := false;
	
END_VAR

(* mydint := mytypevar + 1; *)

mydint := false;

(* mytypevar := 21; *)


END_PROGRAM