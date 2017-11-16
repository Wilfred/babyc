

int mul1(int a, int b) {
    return a * b;
}

int mul2(int a, int b) {
    int r = 0;
    int i = 0;

    while (i < a) {
        {
            int i = 0;
            while (i < b) {
                r = r + 1;
                i = i + 1;
            }
        }
        i = i + 1;
    }

    return r;
}

int main() {

    int a = 5;
    int b = 7;
    if (mul1(5, 7) != mul2(7, 5))
        goto err;
    if (mul1(7, 5) != mul2(5, 7))
        goto err;
    return a + b;
err:
    return 0;
}
