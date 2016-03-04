FUNCTION_BLOCK m1

VAR_INPUT
	a : (hello, bye);
END_VAR

VAR
	b : INT;
END_VAR

b := 40;

case a of
hello : b := 1;
bye   : b := 2;
else
b := 30;
end_case;

END_FUNCTION_BLOCK



PROGRAM testprgm

VAR
	ma : m1;
	mb : m1;
END_VAR

ma(a:=hello);
mb(a:=bye);

END_PROGRAM