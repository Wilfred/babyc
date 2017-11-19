/* BabyC a toy compiler */

int main() {

    int *ptr = _alloca(2 * sizeof(int));
    ptr[0] = 0xff3;
    ptr[1] = 0xfff;
    ptr[1] ^= ptr[0];
    return ptr[1];
}

