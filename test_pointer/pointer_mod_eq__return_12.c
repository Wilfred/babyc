/* BabyC a toy compiler */

int main() {

    int *ptr = _alloca(3 * sizeof(int));
    ptr[0] = 22;
    ptr[1] = 13;
    ptr[0] %= ptr[1];
    ptr[0] += 3;
    return ptr[0];
}

