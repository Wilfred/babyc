

static int s = 1;

int fn(int x) {
    s = s + x;
    return x + 1;
}

int main() {
    int j = (s = s + 1) + fn(2) + (s = s * 3);
    if (s != 12) {
        return 20;
    }
    if (j != 17) {
        return 21;
    }
    return 12;
}
