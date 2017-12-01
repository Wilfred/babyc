/* BabyC a toy compiler */

int foo(int x, int y) {
    return x + y;
}

int bar(int x, int y) {
    return x * y;
}

int main() {
    return bar(foo(2, 1), 4);
}
