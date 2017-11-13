

int main() {
    int a = 1;
    int b = 3;

    while (a < b) {
        a = a + 1;
        if (a >= 4)
            continue;
        b = b * 2;
    }

    return a;
}
