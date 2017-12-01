/* BabyC a toy compiler */

/*

#INPUT:    "01020304"
#EXPECTED: "0203040503040506"

*/

int main() {
    int x = -1;

    int bytes = _read(0, &x, sizeof(x));
    x = x + 0x01010101;
    bytes = bytes + _write(1, &x, sizeof(x));
    x = x + 0x01010101;
    bytes = bytes + _write(1, &x, sizeof(x));
    return bytes;
}
