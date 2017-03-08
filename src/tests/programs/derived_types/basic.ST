TYPE
	base_type : INT;
	t1 : base_type;
	t2 : t1 := 1;
	t3 : t2;

	t4 : INT := 5;
	t5 : t3 := 5;
END_TYPE

VAR_GLOBAL
	v1 : t3;
	v2 : t4;
	v3 : t5;
	v4 : t1;
	v5 : later_defined;
END_VAR

TYPE
	later_defined : t2;
END_TYPE

PROGRAM t

;

END_PROGRAM
