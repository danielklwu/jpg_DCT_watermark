#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>
#include <jerror.h>
#include "image.h"
#include "dct.h"
#include "watermark.h"
#include "attacks.h"

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