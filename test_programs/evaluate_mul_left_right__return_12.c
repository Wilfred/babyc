

static int s = 1;

int fn(int x) {
    s = s + x;
    return x + 1;
}

int main() {
    int j = (s = s + 8) * fn(2);
    if (s != 11) {
        return 20;
    }
    if (j != 27) {
        return 21;
    }
    return 12;
}
