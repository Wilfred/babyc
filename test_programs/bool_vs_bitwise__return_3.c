
int X(int a, int b, int c) {
    int t = (a != 1);
    t = t | (b != 2);
    t = t | (c != 3);
    return t;
}

int Y(int a, int b, int c) {
    return (a != 1) || (b != 2) || (c != 3);
}

int Z(int a, int b, int c) {
    return ((a - 1) | (b - 2) | (c - 3)) != 0;
}

int main() {
    int a = 1;
    int b = 444;
    int c = 3;

    return X(a, b, c) + Y(a, b, c) + Z(a, b, c);
}
