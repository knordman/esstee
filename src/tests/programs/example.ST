FUNCTION_BLOCK m1

VAR_INPUT
	d : INT;
END_VAR

VAR
	a : INT;
	b : INT;
END_VAR

if (1+1)*10>2 then

   if d = 5*6 then
      if true then
      	 b := 5;
      end_if;
   else
      b := 10;
   end_if;

elsif a=1 then
   b := 4;
else
   b := 2;
end_if;


END_FUNCTION_BLOCK



PROGRAM testprgm

VAR
	ma : m1;
	mb : m1;

END_VAR

ma(d:=30);
mb(d:=31);


END_PROGRAM