#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>
#include <jerror.h>

// Holds DCT coefficient arrays
typedef struct {
    jvirt_barray_ptr *coef_arrays;
    j_decompress_ptr cinfo;
} dct_data_t;

// Watermark parameters
#define WATERMARK_STRENGTH 10000
#define MID_FREQ_POSITIONS 6
static const int mid_freq_coords[MID_FREQ_POSITIONS][2] = {
    {2, 1}, {1, 2}, {3, 1}, {1, 3}, {2, 2}, {3, 2}
};

void string_to_binary(const char* str, char* binary);
void embed_bit(JCOEF *coef, int bit);
int embed_watermark(j_decompress_ptr cinfo, jvirt_barray_ptr *coef_arrays, const char* watermark);
dct_data_t* read_jpeg_dct(const char* filename);
int write_jpeg_dct(dct_data_t *dct_data, const char* filename);