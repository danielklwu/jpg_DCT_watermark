#include "main.h"

// Function to convert string to binary
void string_to_binary(const char* str, char* binary) {
    int len = strlen(str);
    int bit_pos = 0;
    
    for (int i = 0; i < len; i++) {
        char c = str[i];
        for (int j = 7; j >= 0; j--) {
            binary[bit_pos++] = ((c >> j) & 1) + '0';
        }
    }
    binary[bit_pos] = '\0';
}

// Function to embed watermark bit into DCT coefficient
void embed_bit(JCOEF *coef, int bit) {
    if (*coef == 0) return; // Skip zero coefficients
    
    int sign = (*coef < 0) ? -1 : 1;
    int abs_coef = abs(*coef);
    
    // Embed bit in LSB with strength adjustment
    if (bit == 1) {
        if ((abs_coef % 2) == 0) {
            abs_coef += WATERMARK_STRENGTH;
        }
    } else {
        if ((abs_coef % 2) == 1) {
            abs_coef += WATERMARK_STRENGTH;
        }
    }
    
    *coef = sign * abs_coef;
}

// Function to embed watermark into DCT coefficients
int embed_watermark(j_decompress_ptr cinfo, jvirt_barray_ptr *coef_arrays, 
                   const char* watermark) {
    char binary_watermark[1024];
    string_to_binary(watermark, binary_watermark);
    int watermark_len = strlen(binary_watermark);
    int bit_index = 0;
    
    printf("Embedding watermark: %s\n", watermark);
    printf("Binary representation: %s\n", binary_watermark);
    
    // Process each component (Y, Cb, Cr)
    for (int comp = 0; comp < cinfo->num_components; comp++) {
        jpeg_component_info *compptr = &cinfo->comp_info[comp];
        
        // Get block dimensions
        JDIMENSION block_row_total = compptr->height_in_blocks;
        JDIMENSION block_col_total = compptr->width_in_blocks;
        
        printf("Component %d: %dx%d blocks\n", comp, 
               block_col_total, block_row_total);
        
        // Process each block row
        for (JDIMENSION block_row = 0; block_row < block_row_total; block_row++) {
            JBLOCKARRAY buffer = (cinfo->mem->access_virt_barray)
                ((j_common_ptr)cinfo, coef_arrays[comp], block_row, 1, TRUE);
            
            // Process each block in the row
            for (JDIMENSION block_col = 0; block_col < block_col_total; block_col++) {
                JBLOCK* block = &buffer[0][block_col];
                
                // Embed watermark bits in mid-frequency coefficients
                for (int pos = 0; pos < MID_FREQ_POSITIONS && bit_index < watermark_len; pos++) {
                    int row = mid_freq_coords[pos][0];
                    int col = mid_freq_coords[pos][1];
                    int coef_index = row * 8 + col;
                    
                    if ((*block)[coef_index] != 0) {
                        int bit = binary_watermark[bit_index] - '0';
                        embed_bit(&(*block)[coef_index], bit);
                        bit_index++;
                    }
                }
            }
        }
    }
    
    printf("Embedded %d bits of watermark data\n", bit_index);
    return bit_index;
}

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