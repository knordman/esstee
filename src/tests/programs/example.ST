PROGRAM testprgm

TYPE

myenum : (one, two, three) := two;

customdint : DINT;

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


END_TYPE

VAR
	mytypevar : myarray;

	myenumone, myenumtwo : myenum;

	mydint : customdint;
END_VAR

(* mydint := mytypevar + 1; *)

;

(* mytypevar := 21; *)


END_PROGRAM