#include "hd.h"
#include <stdio.h>

void hd(const void *data, size_t size) {
    uint8_t ascii[17];
    size_t i, j;
    ascii[16] = '\0';

    for (i = 0; i < size; ++i) {
        printf("%02X ", ((uint8_t *)data)[i]);
        if (((uint8_t *)data)[i] >= ' ' && ((uint8_t *)data)[i] <= '~') {
            ascii[i % 16] = ((uint8_t *)data)[i];
        } else {
            ascii[i % 16] = '.';
        }
        if ((i + 1) % 8 == 0 || i + 1 == size) {
            printf(" ");
            if ((i + 1) % 16 == 0) {
                printf("|  %s \n", ascii);
            } else if (i + 1 == size) {
                ascii[(i + 1) % 16] = '\0';
                if ((i + 1) % 16 <= 8) {
                    printf(" ");
                }
                for (j = (i + 1) % 16; j < 16; ++j) {
                    printf("   ");
                }
                printf("|  %s \n", ascii);
            }
        }
    }
}
