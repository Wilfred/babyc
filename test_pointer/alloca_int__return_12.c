/* BabyC a toy compiler */

int main() {
    int *ptr = _alloca(sizeof(int));
    *ptr = 12;
    return ptr[0];
}
