/* BabyC a toy compiler */

int main() {

    int *ptr = _alloca(3 * sizeof(int));
    ptr[0] = 3;
    ptr[1] = 5;
    ptr[2] = 1;
    ptr[1] += *ptr;
    ptr[0] += ptr[1];
    ptr[0] += ptr[2];
    return ptr[0];
}
