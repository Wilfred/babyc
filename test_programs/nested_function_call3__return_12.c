

int foo(int x, int y, int z) { return x + y + z; }

int bar(int x, int y, int z) { return x * y * z; }

int main() { return bar(1 + 1, foo(1, 1 + 1, 0), 1 + 1); }
