/* BabyC a toy compiler */

int main() {
    int i = 5;
    int j = 7;

    {
        int i = 0;
        i = i + 1;
        j = j - 1;
    }
    {
        int j = 0;
        j = j + 1;
        i = i + 1;
    }

    return i + j;
}
