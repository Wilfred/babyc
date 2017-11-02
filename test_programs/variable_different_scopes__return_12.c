int foo() {
    int x = 1;
    x = x + 1;
    return x;
}

int bar() {
    int x = 5;
    x = x * 2;
    return x;
}

int one() {
    int x = 4;
    x = x - 3;
    return x;
}

int main() { return (foo() + bar()) * one(); }
