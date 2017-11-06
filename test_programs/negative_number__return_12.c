int main() {
    int x = -12;
    if (x > 0) {
        return 0;
    }
    x = 0 - x;
    if (x < 0) {
        return 1;
    }
    return x;
}
