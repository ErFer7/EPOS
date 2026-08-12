#pragma once

const unsigned int CACHE_LINE_SIZE = 64;
const unsigned int L1_CACHE_SIZE = 32 * 1024;
const unsigned int L2_CACHE_SIZE = 2 * 1024 * 1024;
const unsigned int STRIDE = (CACHE_LINE_SIZE / sizeof(unsigned int));

void bandwidth_init(unsigned int *buffer, const unsigned int &size);
void bandwidth_main(unsigned int *buffer, const unsigned int &size);
