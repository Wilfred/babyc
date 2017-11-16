
int foo(int x) {
    if (!x) {
        goto err;
    }

    return 0x1;
err:
    return 0x4;
}

int bar(int x) {
    if (!x) {
        goto err;
    }

    return 0x2;
err:
    return 0x8;
}

int main() {
    return foo(0) + bar(0);
}
