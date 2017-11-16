/* BabyC a toy compiler */

int main() {
    int a = 1;
    int b = 3;

    while (a < b) {
        b = b * 2;
        if (a == 2)
            break;
        a = a + 1;
    }

    return b;
}
