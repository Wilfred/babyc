
/* 
#INPUT: "313233"
#EXPECTED: "313233"
*/

int main()
{
   int c;
   while ( (c = _getc() ) != -1) {
   _putc(c);
   }
   return 0;
}

