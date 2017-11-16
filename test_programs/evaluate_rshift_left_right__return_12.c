/* BabyC a toy compiler */

static int s = 1;

int fn(int x) {
    s = s + x;
    return x + 1;
}

int main() {
    int j = (s = s * 16) >> fn(2);
    if (s != 18) {
        return 20;
    }
    if (j != 2) {
        return 21;
    }
    return 12;
}
