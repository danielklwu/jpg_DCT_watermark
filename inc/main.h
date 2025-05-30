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
#define DISTANCE_D 10  // Distance parameter for coefficient relationships
#define MAX_MODIFICATION_MD 10  // Maximum modification distance for validity check
#define QUALITY_FACTOR_Q 75  // Quality factor for quantization (75%)

// Location sets from paper (using first few sets)
#define NUM_LOCATION_SETS 8
static const int location_sets[NUM_LOCATION_SETS][3][2] = {
    {{0,2}, {1,1}, {1,2}},  // Set 1: positions 2, 9, 10
    {{1,1}, {0,2}, {1,2}},  // Set 2
    {{0,3}, {1,2}, {1,3}},  // Set 3
    {{1,2}, {0,3}, {1,3}},  // Set 4
    {{1,1}, {0,2}, {1,2}},  // Set 5
    {{0,2}, {1,1}, {1,2}},  // Set 6
    {{1,1}, {2,0}, {0,2}},  // Set 7
    {{2,0}, {1,1}, {0,2}}   // Set 8
};

// main.c
void cleanup_dct_data(dct_data_t *dct_data);

// read.c
dct_data_t* read_jpeg_dct(const char* filename);

// write.c
int write_jpeg_dct(dct_data_t *dct_data, const char* filename);

// embed.c
int embed_watermark(j_decompress_ptr cinfo, jvirt_barray_ptr *coef_arrays, const char* watermark);

// extract.c
int extract_watermark(const char* filename, char* watermark, int max_chars);