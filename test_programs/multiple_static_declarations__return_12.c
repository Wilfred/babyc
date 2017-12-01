/* BabyC a toy compiler */
int i, k = 21, j;

int foo() {
    static int i, k = 11, j;
    k = k + 1;
    i = 2;
    j = 1;
    i = i + 1;
    j = j + 1;
    if (i == 3 && j == 2) {
        return k;
    }
    return 1;
}

int main() {
    i = foo();
    k = k + 1;
    j = foo();
    if (i == 12 && j == 13 && k == 22) {
        return 12;
    }
    return 0;
}
