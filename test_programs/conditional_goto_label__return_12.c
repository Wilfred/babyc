int foo(int x, int y) {
    if (!(x + 1 != y - 1)) {
        goto err;
    }

    return 0x1;
err:
    return 0x4;
}

int bar(int x, int y) {
    if (!(x + 1 != y - 1)) {
        goto err;
    }

    return 0x2;
err:
    return 0x8;
}

int main() { return foo(2, 4) + bar(6, 8); }
