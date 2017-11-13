
/*

Square root

   echo 144 | a.out  prints 12
   echo 123000123 | a.out  prints 110090

#INPUT:    "313434"
#EXPECTED: "3132"

*/

/* sqrt binary and recursive */
int my_sqrt(int i)
{
   if (i < 1) return 0;
   int lo = 2 * my_sqrt( i >> 2 );
   int hi = lo + 1;
   if (i < (hi * hi)) return lo;
   return hi;
}


/*---------------------------------------------------------------
* read a string from stdin (atoi function)
*/
int read_string() {
    int v = 0;
    int digit = 0;

    while (1 == _read(0, &digit, 1)) {
        if (digit >= 0x30 && digit <= 0x39) {
            v = v * 10 + digit - 0x30;
        }
    }
    return v;
}

/*---------------------------------------------------------------
* write a string to stdout (itoa function)
*/
int write_string(int n) {
    int digit = 0;
    int t = n;
    int v = 1000000000;

    while (v > 1) {
    if (n >= v) {
        digit = (t / v) + 0x30;
        _write(1, &digit, 1);
        t = t % v;
    }
    v = v / 10;
    }
    digit = t + 0x30;
    _write(1, &digit, 1);
    return 0;
}



int main()
{
   int n = read_string();
   n = my_sqrt(n);
   write_string(n);
   return 0; 
}

