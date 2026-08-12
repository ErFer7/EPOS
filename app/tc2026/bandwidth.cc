#include "bandwidth.h"

void bandwidth_init(unsigned int *buffer, const unsigned int &size) {
    unsigned int allocation_size = size / sizeof(unsigned int);
    buffer = new unsigned int[allocation_size];

    for (unsigned int i = 0; i < allocation_size; i += STRIDE) {
        buffer[i] = i;
    }
}

void bandwidth_main(unsigned int *buffer, const unsigned int &size) {
    unsigned int allocation_size = size / sizeof(unsigned int);

    for (unsigned int i = 0; i < allocation_size; i += STRIDE) {
        buffer[i] = buffer[i] + 1;
    }
}
