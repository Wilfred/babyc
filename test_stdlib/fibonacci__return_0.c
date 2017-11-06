
/* Do not remove this comment !!! INPUT and EXPECTED are needed to run the tests
.....

Buffers of int in little endian representation

fibonacci(0x8) == 0x15
fibonacci(0x9) == 0x22

#INPUT:    "0800000009000000"
#EXPECTED: "1500000022000000"

*/

int fibonacci(int n) {
    int fnow = 0;
    int fnext = 1;

    while (n > 1) {
        int tempf = fnow + fnext;

        fnow = fnext;
        fnext = tempf;
        n = n - 1;
    }
    return fnext;
}

int main() {
    int r;
    int n;

    while (1) {
        r = _read(0, &n, sizeof(n));
        if (r != sizeof(n)) {
            _exit(0);
        }
        r = fibonacci(n);
        _write(1, &r, sizeof(r));
    }
}
