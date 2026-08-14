#pragma once

// TODO: Use 64 bits
class Bandwidth {
   public:
    static const unsigned int CACHE_LINE_SIZE = 64;
    static const unsigned int L1_CACHE_SIZE = 32 * 1024;
    static const unsigned int L2_CACHE_SIZE = 2 * 1024 * 1024;
    static const unsigned int STRIDE = (CACHE_LINE_SIZE / sizeof(unsigned int));

   public:
    Bandwidth(unsigned int cache_size) : _buffer(nullptr), _size(cache_size) {
        unsigned int allocation_size = _size / sizeof(unsigned int);
        _buffer = new unsigned int[allocation_size];

        for (unsigned int i = 0; i < allocation_size; i += STRIDE) {
            _buffer[i] = i;
        }
    }

    ~Bandwidth() { delete[] _buffer; }

    inline int run() {
        unsigned int allocation_size = _size / sizeof(unsigned int);

        for (unsigned int i = 0; i < allocation_size; i += STRIDE) {
            _buffer[i] = _buffer[i] + 1;
        }

        return 0;
    }

   private:
    unsigned int *_buffer;
    unsigned int _size;
};
