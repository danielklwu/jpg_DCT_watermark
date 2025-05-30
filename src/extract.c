#include "main.h"

// Function to convert binary to string
int binary_to_string(const char* binary, char* str, int bit_index){
    int char_count = 0;
    for (int i = 0; i < bit_index && i + 7 < bit_index; i += 8) {
        char c = 0;
        for (int j = 0; j < 8; j++) {
            if (binary[i + j] == '1') {
                c |= (1 << (7 - j));
            }
        }
        if (c >= 32 && c <= 126) {  // Printable ASCII
            str[char_count++] = c;
        } else {
            break;  // End of string or invalid character
        }
    }
    str[char_count] = '\0';
    return char_count;
}

// Function to read embedded bit
int read(JBLOCK block, int location_set_idx) {
    int k1 = location_sets[location_set_idx][0][0];
    int l1 = location_sets[location_set_idx][0][1];
    int k2 = location_sets[location_set_idx][1][0];
    int l2 = location_sets[location_set_idx][1][1];
    int k3 = location_sets[location_set_idx][2][0];
    int l3 = location_sets[location_set_idx][2][1];
    
    JCOEF yq1 = block[k1 * 8 + l1];
    JCOEF yq2 = block[k2 * 8 + l2];
    JCOEF yq3 = block[k3 * 8 + l3];
    
    // Check for bit '1' pattern
    if (yq1 > yq3 + DISTANCE_D && yq2 > yq3 + DISTANCE_D) {
        return 1;
    }
    
    // Check for bit '0' pattern
    if (yq1 + DISTANCE_D < yq3 && yq2 + DISTANCE_D < yq3) {
        return 0;
    }
    
    // Pattern damaged or invalid
    return -1;
}

// Function to extract watermark from DCT coefficients
int extract_watermark(const char* filename, char* watermark, int max_chars) {
    printf("Extracting watermark from: %s\n", filename);
    
    // Read JPEG and get DCT coefficients
    dct_data_t *dct_data = read_jpeg_dct(filename);
    if (!dct_data) {
        fprintf(stderr, "Failed to read DCT coefficients from %s\n", filename);
        return 0;
    }
    
    j_decompress_ptr cinfo = dct_data->cinfo;
    jvirt_barray_ptr *coef_arrays = dct_data->coef_arrays;
    int max_bits = max_chars * 8;

    int bit_index = 0;
    int blocks_processed = 0;
    char binary_result[1024] = {0};
    
    printf("Extracting watermark...\n");
    
    // Process luminance component (Y) first
    for (int comp = 0; comp < cinfo->num_components && bit_index < max_bits; comp++) {
        jpeg_component_info *compptr = &cinfo->comp_info[comp];
        
        // Get block dimensions  
        JDIMENSION block_row_total = compptr->height_in_blocks;
        JDIMENSION block_col_total = compptr->width_in_blocks;

        printf("Component %d: %dx%d blocks\n", comp, 
               block_col_total, block_row_total);
        
        // Process each block row
        for (JDIMENSION block_row = 0; block_row < block_row_total && bit_index < max_bits; block_row++) {
            JBLOCKARRAY buffer = (cinfo->mem->access_virt_barray)
                ((j_common_ptr)cinfo, coef_arrays[comp], block_row, 1, FALSE);
            
            // Process each block in the row
            for (JDIMENSION block_col = 0; block_col < block_col_total && bit_index < max_bits; block_col++) {
                JBLOCK* block = &buffer[0][block_col];
                blocks_processed++;
                
                // Use same location set selection as embedding
                int location_set_idx = blocks_processed % NUM_LOCATION_SETS;
                
                // if (check_block_validity(*block, location_set_idx, bit)) { // to fix
                    int bit = read(*block, location_set_idx);
                    if (bit >= 0) {  // Valid bit extracted
                        binary_result[bit_index] = '0' + bit;
                        bit_index++;
                    }
                // }
            }
        }
    }
    
    binary_result[bit_index] = '\0';
    printf("Extracted binary: %s\n", binary_result);
    
    // Convert binary back to string, count characters converted
    int char_count = binary_to_string(binary_result, watermark, bit_index);
    
    printf("Extracted %d bits, decoded to: '%s'\n", bit_index, watermark);

    // Cleanup
    cleanup_dct_data(dct_data);

    return char_count;
}