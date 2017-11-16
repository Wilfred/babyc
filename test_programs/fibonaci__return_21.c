
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
    return fibb(8);
}
