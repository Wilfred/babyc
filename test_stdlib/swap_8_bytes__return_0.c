

/*

input and output from different length

#INPUT:    "4142434445464748"
#EXPECTED: "4847464544434241"

*/

int swap2(int t) {
    return ((t << 8) & 0xffff) | ((t >> 8) & 0xff);
}

int swap4(int t) {
    return (swap2(t & 0xffff) << 16) | swap2((t >> 16) & 0xffff);
}

int main() {
    int x;
    int y;

    _read(0, &x, sizeof(x));
    _read(0, &y, sizeof(y));
    x = swap4(x);
    y = swap4(y);
    _write(1, &y, sizeof(y));
    _write(1, &x, sizeof(x));
    return 0;
}
