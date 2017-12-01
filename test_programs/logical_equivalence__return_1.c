/* BabyC a toy compiler */

int main() {
    int w = 12;
    int x = 34;

    int e1 = !(w == 12 || x == 34) == (w != 12 && x != 34);
    int e2 = !(w != 12 || x == 34) == (w == 12 && x != 34);
    int e3 = !(w == 12 || x != 34) == (w != 12 && x == 34);
    int e4 = !(w != 12 || x != 34) == (w == 12 && x == 34);

    return (e1 == 1 && e2 == 1 && e3 == 1 && e4 == 1);
}
