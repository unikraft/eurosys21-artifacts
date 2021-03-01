#include <stdio.h>
#include <sys/io.h>
int main(int argc, char **argv) {
    int err;

    err = iopl(3);
    if (err < 0) {
        printf("iopl failed\n");
    }

    err = ioperm(0x03f0, 16, 1);
    if (err < 0) {
        printf("ioperm failed\n");
    }

    outb(123, 0x03f0);

    return 0;
}
