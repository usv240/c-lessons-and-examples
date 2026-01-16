#include <stdio.h>

int square(int x) {
    return x * x; // step into this
}

int main(void) {
    int variable_name = 5;       // we'll print this in gdb
    int result = square(variable_name);
    printf("result=%d\n", result);
    return 0;
}
