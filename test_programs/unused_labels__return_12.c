/* BabyC a toy compiler */

int foo(int x) {
here:
    return 0x4;
}

int bar(int x) {
we:
    return 0x8;
}

int main() {
are:
    return foo(0) + bar(0);
}
