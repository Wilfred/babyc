

int bitwise_test(int x) {
    return ~~~~x;
}

int logical_test(int x) {
    return !!!!x;
}

int arithmetic_test(int x) {
    return -(-(-(-x)));
}

int main() {
    return bitwise_test(5) + arithmetic_test(6) + logical_test(7);
}
