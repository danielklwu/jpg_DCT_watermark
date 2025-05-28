#include "main.h"

// Function to write JPEG with modified DCT coefficients
int write_jpeg_dct(dct_data_t *dct_data, const char* filename) {
    FILE *outfile;
    struct jpeg_compress_struct cinfo_out;
    struct jpeg_error_mgr jerr_out;
    
    // Open output file
    if ((outfile = fopen(filename, "wb")) == NULL) {
        fprintf(stderr, "Can't open output file %s\n", filename);
        return 0;
    }
    
    // Initialize JPEG compression
    cinfo_out.err = jpeg_std_error(&jerr_out);
    jpeg_create_compress(&cinfo_out);
    jpeg_stdio_dest(&cinfo_out, outfile);
    
    // Copy compression parameters from original
    jpeg_copy_critical_parameters(dct_data->cinfo, &cinfo_out);
    
    // Write coefficients
    jpeg_write_coefficients(&cinfo_out, dct_data->coef_arrays);
    
    // Finish compression
    jpeg_finish_compress(&cinfo_out);
    jpeg_destroy_compress(&cinfo_out);
    fclose(outfile);
    
    return 1;
}