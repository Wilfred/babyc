/* BabyC a toy compiler */

int main() {

    int *ptr = _alloca(2 * sizeof(int));
    ptr[0] = 17;
    ptr[1] = 4;
    ptr[0] -= ptr[1];
    ptr[0] -= 1;
    return ptr[0];
}
