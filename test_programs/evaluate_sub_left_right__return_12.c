/* BabyC a toy compiler */

static int s = 1;

int fn(int x) {
    s = s + x;
    return x + 1;
}

int main() {
    int j = (s = s * 7) - fn(2) - (s = s / 3);
    if (s != 3) {
        return 20;
    }
    if (j != 1) {
        return 21;
    }
    return 12;
}
