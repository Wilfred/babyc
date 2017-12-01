

int abs(int x) {
    int r = ({
        int y = x;
        int z;
        if (y > 0)
            z = y;
        else
            z = -y;
        z;
    });
    return r;
}

int main() {
    return abs(-12);
}
