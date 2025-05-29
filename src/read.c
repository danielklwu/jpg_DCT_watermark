#include "main.h"

// Read JPEG and get DCT coefficients
dct_data_t* read_jpeg_dct(const char* filename) {
    FILE *infile;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    dct_data_t *dct_data;
    
    // Open input file
    if ((infile = fopen(filename, "rb")) == NULL) {
        fprintf(stderr, "Can't open input file %s\n", filename);
        return NULL;
    }
    
    // Allocate DCT data structure
    dct_data = (dct_data_t*)malloc(sizeof(dct_data_t));
    if (!dct_data) {
        fclose(infile);
        return NULL;
    }
    
    // Initialize JPEG decompression
    dct_data->cinfo = (j_decompress_ptr)malloc(sizeof(struct jpeg_decompress_struct));
    dct_data->cinfo->err = jpeg_std_error(&jerr);
    jpeg_create_decompress(dct_data->cinfo);
    jpeg_stdio_src(dct_data->cinfo, infile);
    
    // Read JPEG header
    jpeg_read_header(dct_data->cinfo, TRUE);
    
    // Get DCT coefficients
    dct_data->coef_arrays = jpeg_read_coefficients(dct_data->cinfo);
    
    fclose(infile);
    return dct_data;
}