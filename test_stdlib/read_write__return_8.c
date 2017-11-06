

/*

input and output from different length

#INPUT:    "0102030401010101"
#EXPECTED: "02030405"

*/

int main() {
    int x = -1;
    int y = +1;

    int bytes = _read(0, &x, sizeof(x));
    bytes = bytes + _read(0, &y, sizeof(y));
    int z = x + y;
    _write(1, &z, sizeof(z));
    return bytes;
}
