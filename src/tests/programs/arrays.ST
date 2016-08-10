VAR_GLOBAL
	arr1 : ARRAY [2..6] of INT;
	arr2 : ARRAY [-10..0] of INT;
	arr3 : ARRAY [0..1,0..2,0..3] of INT;
	arr4 : ARRAY [1..2,1..4,1..3] of INT :=
	     [
		[[1,2,3],[4,5,6],[7,8,9],[10,11,12]],
		[[21,22,23],[31,32,33],[34,35,36],[37,38,39]]
	     ];
END_VAR

PROGRAM t

;

END_PROGRAM
