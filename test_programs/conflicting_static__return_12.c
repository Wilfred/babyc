

static int xxx = 2;

int foo() {
    static int xxx = 0;
    int yyy = 1;

    return xxx + yyy;
}

int bar() {
    static int xxx = 2;
    int yyy = 1;

    return xxx * (yyy + yyy);
}

int main() { return (xxx + foo()) * bar(); }
