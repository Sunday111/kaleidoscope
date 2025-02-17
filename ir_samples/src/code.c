#include "stdio.h"

int g(unsigned a, int b, int c) {
    return a + b * c;
}

int main() {
    printf("%d\n", g(1, 2, 3)); // NOLINT
    return 0;
}
