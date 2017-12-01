/* BabyC a toy compiler */

int main() {

    int *ptr = _alloca(2 * sizeof(int));
    ptr[0] = 9;
    ptr[1] = 2;
    if (*ptr != 9) {
        return 1;
    }
    if (ptr[1] != 2) {
        return 1;
    }
    if (ptr[0] != 9) {
        return 1;
    }
    *ptr = *ptr + 1;
    if (ptr[1] != 2) {
        return 1;
    }
    return ptr[0] + ptr[1];
}
