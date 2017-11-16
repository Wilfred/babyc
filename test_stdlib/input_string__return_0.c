/* BabyC a toy compiler */

/* read bytes from stdin and convert to number (atoi from C library) */

/*

echo 65535 | ./a.out | od -t x1

#INPUT:     "3635353335"
#EXPECTED:  "ffff0000"

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

int main() {
    int value = read_string();
    _write(1, &value, sizeof(value));
    return 0;
}
