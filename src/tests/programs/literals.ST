PROGRAM t

VAR
	i1 : DINT;
	i2 : DINT;
	i3 : DINT;
	i4 : DINT;

	t1 : TIME;

	r1 : REAL;
	r2 : REAL;

	b1 : BOOL;
	b2 : BOOL;

	td : TIME_OF_DAY;
	d1 : DATE;
	dd : DATE_AND_TIME;

	str : STRING;
	wstr : WSTRING;
END_VAR

;

END_PROGRAM

PROGRAM t_init

VAR
	i1 : DINT := 1;
	i2 : DINT := 2;
	i3 : DINT := 3;
	i4 : DINT := 4;

	t1 : TIME := T#1s;

	r1 : REAL := 1.0;
	r2 : REAL := 2.0;

	b1 : BOOL := true;
	b2 : BOOL := false;

	td : TIME_OF_DAY := TOD#10:10:10.10;
	d1 : DATE := D#1234-01-02;
	dd : DATE_AND_TIME := DT#1234-01-01-10:10:10.10;

	str : STRING := 'single';
	wstr : WSTRING := "double";
END_VAR

;

END_PROGRAM
