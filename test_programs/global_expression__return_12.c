

int main() {
    static int w = (15 * 15 - 200); /* 25 */
    static int ww = w - 1;          /* 24 */
    int www = ww / 2;               /* 12 */
    return www;
}
