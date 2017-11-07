
int fn(int x) { return x * 10; }

int main() {
    int i = fn(11) >> fn(13);
    return i == 0;
}
