#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "watermark.h"
#include "dct.h"

void generate_sequence(int *sequence, int length, int seed) {
    srand(seed);
    for (int i = 0; i < length; i++) {
        sequence[i] = rand() % length;
    }
}

void embed_watermark(Image *img, char *watermark, int watermark_length, double alpha) {
    int blocks_x = img->width / BLOCK_SIZE;
    int blocks_y = img->height / BLOCK_SIZE;
    int total_blocks = blocks_x * blocks_y;
    
    int *block_sequence = (int*)malloc(total_blocks * sizeof(int));
    generate_sequence(block_sequence, total_blocks, 12345);
    
    double block[BLOCK_SIZE][BLOCK_SIZE];
    double dct_block[BLOCK_SIZE][BLOCK_SIZE];
    
    int watermark_bit = 0;
    
    for (int block_idx = 0; block_idx < total_blocks && watermark_bit < watermark_length; block_idx++) {
        int selected_block = block_sequence[block_idx];
        int block_y = selected_block / blocks_x;
        int block_x = selected_block % blocks_x;
        
        // Extract block
        for (int i = 0; i < BLOCK_SIZE; i++) {
            for (int j = 0; j < BLOCK_SIZE; j++) {
                int y = block_y * BLOCK_SIZE + i;
                int x = block_x * BLOCK_SIZE + j;
                if (y < img->height && x < img->width) {
                    block[i][j] = (double)img->data[y][x];
                } else {
                    block[i][j] = 0.0;
                }
            }
        }
        
        forward_dct(block, dct_block);
        
        int bit = (watermark[watermark_bit / 8] >> (7 - (watermark_bit % 8))) & 1;
        
        if (bit == 1) {
            if (dct_block[3][4] <= dct_block[4][3]) {
                double avg = (dct_block[3][4] + dct_block[4][3]) / 2.0;
                dct_block[3][4] = avg + alpha;
                dct_block[4][3] = avg - alpha;
            }
        } else {
            if (dct_block[4][3] <= dct_block[3][4]) {
                double avg = (dct_block[3][4] + dct_block[4][3]) / 2.0;
                dct_block[4][3] = avg + alpha;
                dct_block[3][4] = avg - alpha;
            }
        }
        
        inverse_dct(dct_block, block);
        
        for (int i = 0; i < BLOCK_SIZE; i++) {
            for (int j = 0; j < BLOCK_SIZE; j++) {
                int y = block_y * BLOCK_SIZE + i;
                int x = block_x * BLOCK_SIZE + j;
                if (y < img->height && x < img->width) {
                    int pixel_val = (int)round(block[i][j]);
                    if (pixel_val < 0) pixel_val = 0;
                    if (pixel_val > 255) pixel_val = 255;
                    img->data[y][x] = (unsigned char)pixel_val;
                }
            }
        }
        
        watermark_bit++;
    }
    
    free(block_sequence);
}

void extract_watermark(Image *img, char *extracted_watermark, int watermark_length) {
    int blocks_x = img->width / BLOCK_SIZE;
    int blocks_y = img->height / BLOCK_SIZE;
    int total_blocks = blocks_x * blocks_y;
    
    int *block_sequence = (int*)malloc(total_blocks * sizeof(int));
    generate_sequence(block_sequence, total_blocks, 12345);
    
    double block[BLOCK_SIZE][BLOCK_SIZE];
    double dct_block[BLOCK_SIZE][BLOCK_SIZE];
    
    int watermark_bytes = (watermark_length + 7) / 8;
    memset(extracted_watermark, 0, watermark_bytes);
    
    int watermark_bit = 0;
    
    for (int block_idx = 0; block_idx < total_blocks && watermark_bit < watermark_length; block_idx++) {
        int selected_block = block_sequence[block_idx];
        int block_y = selected_block / blocks_x;
        int block_x = selected_block % blocks_x;
        
        for (int i = 0; i < BLOCK_SIZE; i++) {
            for (int j = 0; j < BLOCK_SIZE; j++) {
                int y = block_y * BLOCK_SIZE + i;
                int x = block_x * BLOCK_SIZE + j;
                if (y < img->height && x < img->width) {
                    block[i][j] = (double)img->data[y][x];
                } else {
                    block[i][j] = 0.0;
                }
            }
        }
        
        forward_dct(block, dct_block);
        
        int bit = (dct_block[3][4] > dct_block[4][3]) ? 1 : 0;
        
        if (bit) {
            extracted_watermark[watermark_bit / 8] |= (1 << (7 - (watermark_bit % 8)));
        }
        
        watermark_bit++;
    }
    
    free(block_sequence);
}

double calculate_similarity(char *watermark1, char *watermark2, int length) {
    int matches = 0;
    int total_bits = 0;
    
    for (int i = 0; i < (length + 7) / 8; i++) {
        for (int j = 0; j < 8 && total_bits < length; j++) {
            int bit1 = (watermark1[i] >> (7 - j)) & 1;
            int bit2 = (watermark2[i] >> (7 - j)) & 1;
            
            if (bit1 == bit2) matches++;
            total_bits++;
        }
    }
    
    return (double)matches / total_bits;
}
