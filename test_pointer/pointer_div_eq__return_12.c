/* BabyC a toy compiler */

int main() {

    int *ptr = _alloca(2 * sizeof(int));
    ptr[0] = 20;
    ptr[1] = 2;
    ptr[0] /= ptr[1];
    ptr[0] += ptr[1];
    return ptr[0];
}

