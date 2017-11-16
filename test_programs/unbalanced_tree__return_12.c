/* BabyC a toy compiler */

int one() {
    return 1;
}

int two() {
    return one() + one();
}

int bar(int i, int j) {
    return 5 + (((i + j) + one()) % ((i - j) * two())) + 4;
}

int main() {
    return bar(13, 5);
}
