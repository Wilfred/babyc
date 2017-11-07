
int fibb(int n) {
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
    int n;

    _read(0, &n, sizeof(n));
    int r = fibb(n);

    _write(1, &r, sizeof(r));
}
