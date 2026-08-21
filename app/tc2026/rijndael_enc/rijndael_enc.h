#pragma once

#include "aes.h"
#include "rijndael_enc_libc.h"

class RijndaelEnc {
   public:
    RijndaelEnc();

    ~RijndaelEnc() = default;

    int run();

   private:
    void rijndael_enc_fillrand(unsigned char *buf, int len);
    void rijndael_enc_encfile(struct rijndael_enc_FILE *fin, struct aes *ctx);

   private:
    unsigned char rijndael_enc_key[32];
    int rijndael_enc_key_len;
    rijndael_enc_FILE rijndael_enc_fin;
    int rijndael_enc_checksum = 0;
    unsigned char rijndael_enc_data[];
    static const unsigned char rijndael_enc_input_data[];
};
