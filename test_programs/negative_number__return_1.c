/* Babyc Toy compiler */

int main() {

    int a = 254 - !~0;
    if (a < 0)
        return 2;
    return 1;
}
