#ifndef RIJNDAEL_ENC_LIBC_H
#define RIJNDAEL_ENC_LIBC_H

int rijndael_enc_toupper(int c);

enum RijndaelOrigin { RIJNDAEL_ENC_SEEK_SET, RIJNDAEL_ENC_SEEK_CUR, RIJNDAEL_ENC_SEEK_END };
struct rijndael_enc_FILE {
    unsigned char *data;
    unsigned long size;
    unsigned cur_pos;
};

unsigned long rijndael_enc_fread(void *ptr, unsigned long size, unsigned long count, struct rijndael_enc_FILE *stream);
unsigned long rijndael_enc_fwrite(const void *ptr,
                                  unsigned long size,
                                  unsigned long count,
                                  struct rijndael_enc_FILE *stream);
int rijndael_enc_fseek(struct rijndael_enc_FILE *stream, long int offset, RijndaelOrigin origin);
int rijndael_enc_fgetpos(struct rijndael_enc_FILE *stream, unsigned *position);
int rijndael_enc_feof(struct rijndael_enc_FILE *stream);

#endif  // RIJNDAEL_ENC_LIBC_H
