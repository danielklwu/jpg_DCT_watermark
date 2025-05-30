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

// Function to check if a block is valid for embedding based on the paper's method
int check_write(JBLOCK block, int location_set_idx, int bit_to_embed) {
    int k1 = location_sets[location_set_idx][0][0];
    int l1 = location_sets[location_set_idx][0][1];
    int k2 = location_sets[location_set_idx][1][0]; 
    int l2 = location_sets[location_set_idx][1][1];
    int k3 = location_sets[location_set_idx][2][0];
    int l3 = location_sets[location_set_idx][2][1];
    
    JCOEF yq1 = block[k1 * 8 + l1];
    JCOEF yq2 = block[k2 * 8 + l2];
    JCOEF yq3 = block[k3 * 8 + l3];
    
    int abs_yq1 = abs(yq1);
    int abs_yq2 = abs(yq2);
    int abs_yq3 = abs(yq3);
    
    if (bit_to_embed == 1) {
        // For bit '1': check if MIN(|YQ(k1,l1)|, |YQ(k2,l2)|) + MD >= |YQ(k3,l3)|
        int min_val = (abs_yq1 < abs_yq2) ? abs_yq1 : abs_yq2;
        return (min_val + MAX_MODIFICATION_MD >= abs_yq3);
    } else {
        // For bit '0': check if MAX(|YQ(k1,l1)|, |YQ(k2,l2)|) + MD <= |YQ(k3,l3)|
        int max_val = (abs_yq1 > abs_yq2) ? abs_yq1 : abs_yq2;
        return (max_val <= abs_yq3 + MAX_MODIFICATION_MD);
    }
}

// Function to embed bit using the three-coefficient relationship
void write(JBLOCK block, int location_set_idx, int bit) {
    int k1 = location_sets[location_set_idx][0][0];
    int l1 = location_sets[location_set_idx][0][1];
    int k2 = location_sets[location_set_idx][1][0];
    int l2 = location_sets[location_set_idx][1][1]; 
    int k3 = location_sets[location_set_idx][2][0];
    int l3 = location_sets[location_set_idx][2][1];
    
    JCOEF *yq1 = &block[k1 * 8 + l1];
    JCOEF *yq2 = &block[k2 * 8 + l2];
    JCOEF *yq3 = &block[k3 * 8 + l3];
    
    if (bit == 1) {
        // For bit '1': YQ(k1,l1) > YQ(k3,l3) + D and YQ(k2,l2) > YQ(k3,l3) + D
        if (*yq1 <= *yq3 + DISTANCE_D) {
            *yq1 = *yq3 + DISTANCE_D + 1;
        }
        if (*yq2 <= *yq3 + DISTANCE_D) {
            *yq2 = *yq3 + DISTANCE_D + 1;
        }
    } else {
        // For bit '0': YQ(k1,l1) + D < YQ(k3,l3) and YQ(k2,l2) + D < YQ(k3,l3)
        if (*yq1 + DISTANCE_D >= *yq3) {
            *yq3 = *yq1 + DISTANCE_D + 1;
        }
        if (*yq2 + DISTANCE_D >= *yq3) {
            *yq3 = (*yq2 > *yq1 ? *yq2 : *yq1) + DISTANCE_D + 1;
        }
    }
}

// Function to embed watermark into DCT coefficients
int embed_watermark(j_decompress_ptr cinfo, jvirt_barray_ptr *coef_arrays, 
                   const char* watermark) {
    char binary_watermark[1024];
    string_to_binary(watermark, binary_watermark);
    int watermark_len = strlen(binary_watermark);
    int bit_index = 0;
    int blocks_processed = 0;
    int valid_blocks = 0;
    
    printf("Embedding watermark: %s\n", watermark);
    printf("Binary representation: %s\n", binary_watermark);
    
    // Process luminance component (Y) first for better robustness
    for (int comp = 0; comp < cinfo->num_components && bit_index < watermark_len; comp++) {
        jpeg_component_info *compptr = &cinfo->comp_info[comp];
        
        // Get block dimensions
        JDIMENSION block_row_total = compptr->height_in_blocks;
        JDIMENSION block_col_total = compptr->width_in_blocks;
        
        printf("Component %d: %dx%d blocks\n", comp, 
               block_col_total, block_row_total);
        
        // Process each block row
        for (JDIMENSION block_row = 0; block_row < block_row_total && bit_index < watermark_len; block_row++) {
            JBLOCKARRAY buffer = (cinfo->mem->access_virt_barray)
                ((j_common_ptr)cinfo, coef_arrays[comp], block_row, 1, TRUE);
            
            // Process each block in the row
            for (JDIMENSION block_col = 0; block_col < block_col_total && bit_index < watermark_len; block_col++) {
                JBLOCK* block = &buffer[0][block_col];
                blocks_processed++;
                
                // Use simple round-robin selection of location sets for now
                // Note: embedding blocks in order, could be easily cropped out.
                int location_set_idx = blocks_processed % NUM_LOCATION_SETS;
                int bit = binary_watermark[bit_index] - '0';
                
                // Check if block is valid for embedding this bit
                if (check_write(*block, location_set_idx, bit)) {
                    write(*block, location_set_idx, bit);
                    bit_index++;
                    valid_blocks++;
                    
                    if (bit_index % 8 == 0) {
                        printf("Embedded %d bits (%d characters)\n", bit_index, bit_index/8);
                    }
                } else {
                    // Block invalid
                    printf("Invalid block found!\n");
                }
            }
        }
    }
    
    printf("Embedded %d bits of watermark data in %d valid blocks out of %d total blocks\n", 
           bit_index, valid_blocks, blocks_processed);
    return bit_index;
}
