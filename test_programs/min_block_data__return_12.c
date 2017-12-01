

int min(int x, int y) {
    int r = ({
        int z, xx = x, yy = y;
        if (xx < yy)
            z = xx;
        else
            z = yy;
        z;
    });
    return r;
}

int main() {
    return min(12, 18);
}
