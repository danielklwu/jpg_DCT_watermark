#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <jpeglib.h>
#include <jerror.h>

#define BLOCK_SIZE 8
#define PI 3.14159265359

// Structure to hold image data
typedef struct {
    unsigned char **data;
    int width;
    int height;
} Image;

// DCT coefficient matrix
double dct_coeff[BLOCK_SIZE][BLOCK_SIZE];
double idct_coeff[BLOCK_SIZE][BLOCK_SIZE];

// Initialize DCT coefficient matrices
void init_dct_tables() {
    int i, j;
    double alpha_i, alpha_j;
    
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            alpha_i = (i == 0) ? sqrt(1.0/BLOCK_SIZE) : sqrt(2.0/BLOCK_SIZE);
            alpha_j = (j == 0) ? sqrt(1.0/BLOCK_SIZE) : sqrt(2.0/BLOCK_SIZE);
            
            dct_coeff[i][j] = alpha_i * alpha_j * 
                cos((2*i + 1) * PI * i / (2.0 * BLOCK_SIZE)) *
                cos((2*j + 1) * PI * j / (2.0 * BLOCK_SIZE));
            
            idct_coeff[i][j] = dct_coeff[i][j];
        }
    }
}

// Forward DCT transform for 8x8 block
void forward_dct(double input[BLOCK_SIZE][BLOCK_SIZE], double output[BLOCK_SIZE][BLOCK_SIZE]) {
    int u, v, i, j;
    double sum;
    double alpha_u, alpha_v;
    
    for (u = 0; u < BLOCK_SIZE; u++) {
        for (v = 0; v < BLOCK_SIZE; v++) {
            sum = 0.0;
            alpha_u = (u == 0) ? sqrt(1.0/BLOCK_SIZE) : sqrt(2.0/BLOCK_SIZE);
            alpha_v = (v == 0) ? sqrt(1.0/BLOCK_SIZE) : sqrt(2.0/BLOCK_SIZE);
            
            for (i = 0; i < BLOCK_SIZE; i++) {
                for (j = 0; j < BLOCK_SIZE; j++) {
                    sum += input[i][j] * 
                           cos((2*i + 1) * PI * u / (2.0 * BLOCK_SIZE)) *
                           cos((2*j + 1) * PI * v / (2.0 * BLOCK_SIZE));
                }
            }
            output[u][v] = alpha_u * alpha_v * sum;
        }
    }
}

// Inverse DCT transform for 8x8 block
void inverse_dct(double input[BLOCK_SIZE][BLOCK_SIZE], double output[BLOCK_SIZE][BLOCK_SIZE]) {
    int i, j, u, v;
    double sum;
    double alpha_u, alpha_v;
    
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            sum = 0.0;
            
            for (u = 0; u < BLOCK_SIZE; u++) {
                for (v = 0; v < BLOCK_SIZE; v++) {
                    alpha_u = (u == 0) ? sqrt(1.0/BLOCK_SIZE) : sqrt(2.0/BLOCK_SIZE);
                    alpha_v = (v == 0) ? sqrt(1.0/BLOCK_SIZE) : sqrt(2.0/BLOCK_SIZE);
                    
                    sum += alpha_u * alpha_v * input[u][v] *
                           cos((2*i + 1) * PI * u / (2.0 * BLOCK_SIZE)) *
                           cos((2*j + 1) * PI * v / (2.0 * BLOCK_SIZE));
                }
            }
            output[i][j] = sum;
        }
    }
}

// Allocate memory for image
Image* create_image(int width, int height) {
    Image *img = (Image*)malloc(sizeof(Image));
    img->width = width;
    img->height = height;
    
    img->data = (unsigned char**)malloc(height * sizeof(unsigned char*));
    for (int i = 0; i < height; i++) {
        img->data[i] = (unsigned char*)malloc(width * sizeof(unsigned char));
    }
    
    return img;
}

// Free image memory
void free_image(Image *img) {
    for (int i = 0; i < img->height; i++) {
        free(img->data[i]);
    }
    free(img->data);
    free(img);
}

// Generate pseudo-random sequence for watermark positions
void generate_sequence(int *sequence, int length, int seed) {
    srand(seed);
    for (int i = 0; i < length; i++) {
        sequence[i] = rand() % length;
    }
}

// Koch-Zhao watermark embedding
void embed_watermark(Image *img, char *watermark, int watermark_length, double alpha) {
    int blocks_x = img->width / BLOCK_SIZE;
    int blocks_y = img->height / BLOCK_SIZE;
    int total_blocks = blocks_x * blocks_y;
    
    // Generate random sequence for block selection
    int *block_sequence = (int*)malloc(total_blocks * sizeof(int));
    generate_sequence(block_sequence, total_blocks, 12345); // Fixed seed
    
    double block[BLOCK_SIZE][BLOCK_SIZE];
    double dct_block[BLOCK_SIZE][BLOCK_SIZE];
    
    int watermark_bit = 0;
    
    for (int block_idx = 0; block_idx < total_blocks && watermark_bit < watermark_length; block_idx++) {
        int selected_block = block_sequence[block_idx];
        int block_y = selected_block / blocks_x;
        int block_x = selected_block % blocks_x;
        
        // Extract 8x8 block
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
        
        // Forward DCT
        forward_dct(block, dct_block);
        
        // Koch-Zhao modification: modify specific DCT coefficients
        // We use positions (3,4) and (4,3) as in the original paper
        int bit = (watermark[watermark_bit / 8] >> (7 - (watermark_bit % 8))) & 1;
        
        if (bit == 1) {
            // Make dct_block[3][4] > dct_block[4][3]
            if (dct_block[3][4] <= dct_block[4][3]) {
                double avg = (dct_block[3][4] + dct_block[4][3]) / 2.0;
                dct_block[3][4] = avg + alpha;
                dct_block[4][3] = avg - alpha;
            }
        } else {
            // Make dct_block[4][3] > dct_block[3][4]
            if (dct_block[4][3] <= dct_block[3][4]) {
                double avg = (dct_block[3][4] + dct_block[4][3]) / 2.0;
                dct_block[4][3] = avg + alpha;
                dct_block[3][4] = avg - alpha;
            }
        }
        
        // Inverse DCT
        inverse_dct(dct_block, block);
        
        // Put block back into image
        for (int i = 0; i < BLOCK_SIZE; i++) {
            for (int j = 0; j < BLOCK_SIZE; j++) {
                int y = block_y * BLOCK_SIZE + i;
                int x = block_x * BLOCK_SIZE + j;
                if (y < img->height && x < img->width) {
                    int pixel_val = (int)round(block[i][j]);
                    // Clamp to valid range
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

// Koch-Zhao watermark extraction
void extract_watermark(Image *img, char *extracted_watermark, int watermark_length) {
    int blocks_x = img->width / BLOCK_SIZE;
    int blocks_y = img->height / BLOCK_SIZE;
    int total_blocks = blocks_x * blocks_y;
    
    // Generate same random sequence used for embedding
    int *block_sequence = (int*)malloc(total_blocks * sizeof(int));
    generate_sequence(block_sequence, total_blocks, 12345); // Same seed
    
    double block[BLOCK_SIZE][BLOCK_SIZE];
    double dct_block[BLOCK_SIZE][BLOCK_SIZE];
    
    // Initialize extracted watermark
    int watermark_bytes = (watermark_length + 7) / 8;
    memset(extracted_watermark, 0, watermark_bytes);
    
    int watermark_bit = 0;
    
    for (int block_idx = 0; block_idx < total_blocks && watermark_bit < watermark_length; block_idx++) {
        int selected_block = block_sequence[block_idx];
        int block_y = selected_block / blocks_x;
        int block_x = selected_block % blocks_x;
        
        // Extract 8x8 block
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
        
        // Forward DCT
        forward_dct(block, dct_block);
        
        // Extract bit based on comparison
        int bit = (dct_block[3][4] > dct_block[4][3]) ? 1 : 0;
        
        // Set bit in extracted watermark
        if (bit) {
            extracted_watermark[watermark_bit / 8] |= (1 << (7 - (watermark_bit % 8)));
        }
        
        watermark_bit++;
    }
    
    free(block_sequence);
}

// Simple function to create a test image (gradient pattern)
void create_test_image(Image *img) {
    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            img->data[i][j] = (unsigned char)((i + j) % 256);
        }
    }
}

// Calculate similarity between two watermarks
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

// Save image as JPEG using libjpeg
int save_jpeg(Image *img, const char *filename, int quality) {
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *outfile;
    JSAMPROW row_pointer[1];
    int row_stride;
    
    // Initialize JPEG compression object
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    
    // Open output file
    if ((outfile = fopen(filename, "wb")) == NULL) {
        printf("Error: Cannot create JPEG file %s\n", filename);
        jpeg_destroy_compress(&cinfo);
        return 0;
    }
    
    // Set output file
    jpeg_stdio_dest(&cinfo, outfile);
    
    // Set image parameters
    cinfo.image_width = img->width;
    cinfo.image_height = img->height;
    cinfo.input_components = 1; // Grayscale
    cinfo.in_color_space = JCS_GRAYSCALE;
    
    // Set default compression parameters
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    
    // Start compression
    jpeg_start_compress(&cinfo, TRUE);
    
    // Write scanlines
    row_stride = img->width;
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &img->data[cinfo.next_scanline][0];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }
    
    // Finish compression
    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    
    // Clean up
    jpeg_destroy_compress(&cinfo);
    
    printf("Saved JPEG image as %s (quality: %d)\n", filename, quality);
    return 1;
}

// Load JPEG image using libjpeg
Image* load_jpeg(const char *filename) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *infile;
    JSAMPARRAY buffer;
    int row_stride;
    Image *img;
    
    // Open input file
    if ((infile = fopen(filename, "rb")) == NULL) {
        printf("Error: Cannot open JPEG file %s\n", filename);
        return NULL;
    }
    
    // Initialize JPEG decompression object
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    
    // Set input file
    jpeg_stdio_src(&cinfo, infile);
    
    // Read file parameters
    jpeg_read_header(&cinfo, TRUE);
    
    // Convert to grayscale if needed
    if (cinfo.jpeg_color_space != JCS_GRAYSCALE) {
        cinfo.out_color_space = JCS_GRAYSCALE;
    }
    
    // Start decompression
    jpeg_start_decompress(&cinfo);
    
    // Create image structure
    img = create_image(cinfo.output_width, cinfo.output_height);
    
    // Allocate memory for one row
    row_stride = cinfo.output_width * cinfo.output_components;
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
    
    // Read scanlines
    int row = 0;
    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        memcpy(img->data[row], buffer[0], cinfo.output_width);
        row++;
    }
    
    // Finish decompression
    jpeg_finish_decompress(&cinfo);
    fclose(infile);
    
    // Clean up
    jpeg_destroy_decompress(&cinfo);
    
    printf("Loaded JPEG image from %s (%dx%d)\n", filename, img->width, img->height);
    return img;
}

// Copy image data
Image* copy_image(Image *src) {
    Image *dst = create_image(src->width, src->height);
    for (int i = 0; i < src->height; i++) {
        memcpy(dst->data[i], src->data[i], src->width);
    }
    return dst;
}

// Add noise to image
void add_noise(Image *img, int noise_level) {
    srand(54321); // Fixed seed for reproducibility
    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            int noise = (rand() % (2 * noise_level + 1)) - noise_level;
            int new_val = (int)img->data[i][j] + noise;
            if (new_val < 0) new_val = 0;
            if (new_val > 255) new_val = 255;
            img->data[i][j] = (unsigned char)new_val;
        }
    }
}

// Main demonstration function
int main() {
    printf("Koch-Zhao DCT Watermarking Algorithm Demo\n");
    printf("=========================================\n\n");
    
    // Initialize DCT tables
    init_dct_tables();
    
    // Create test image
    Image *original = create_image(256, 256);
    create_test_image(original);
    printf("Created test image: %dx%d pixels\n", original->width, original->height);
    
    // Save original test image
    save_jpeg(original, "original_test_image.jpg", 90);
    
    // Create a copy for watermarking
    Image *watermarked = copy_image(original);
    
    // Create watermark
    char watermark[] = "WATERMARK_TEST_123";
    int watermark_length = strlen(watermark) * 8; // Convert to bits
    printf("Original watermark: \"%s\" (%d bits)\n", watermark, watermark_length);
    
    // Embed watermark
    double alpha = 50.0; // Embedding strength
    printf("Embedding watermark with strength alpha = %.1f\n", alpha);
    embed_watermark(watermarked, watermark, watermark_length, alpha);
    printf("Watermark embedded successfully!\n");
    
    // Save watermarked image
    save_jpeg(watermarked, "watermarked_image.jpg", 90);
    
    // Extract watermark from watermarked image
    char extracted_watermark[(strlen(watermark) + 7) / 8 + 1];
    memset(extracted_watermark, 0, sizeof(extracted_watermark));
    
    printf("\nExtracting watermark from clean watermarked image...\n");
    extract_watermark(watermarked, extracted_watermark, watermark_length);
    
    // Convert extracted bits back to string
    char extracted_string[strlen(watermark) + 1];
    memset(extracted_string, 0, sizeof(extracted_string));
    
    for (int i = 0; i < strlen(watermark); i++) {
        char c = 0;
        for (int j = 0; j < 8; j++) {
            int bit = (extracted_watermark[i] >> (7 - j)) & 1;
            c |= (bit << (7 - j));
        }
        extracted_string[i] = c;
    }
    
    printf("Extracted watermark: \"%s\"\n", extracted_string);
    
    // Calculate similarity
    double similarity = calculate_similarity(watermark, extracted_watermark, watermark_length);
    printf("Bit-level similarity: %.2f%% (%d/%d bits match)\n", 
           similarity * 100, (int)(similarity * watermark_length), watermark_length);
    
    // Test robustness with noise
    printf("\nTesting robustness with noise...\n");
    Image *noisy = copy_image(watermarked);
    add_noise(noisy, 10); // Add ±10 noise
    
    // Save noisy image
    save_jpeg(noisy, "noisy_watermarked_image.jpg", 90);
    
    // Extract watermark from noisy image
    memset(extracted_watermark, 0, sizeof(extracted_watermark));
    extract_watermark(noisy, extracted_watermark, watermark_length);
    
    // Convert to string again
    memset(extracted_string, 0, sizeof(extracted_string));
    for (int i = 0; i < strlen(watermark); i++) {
        char c = 0;
        for (int j = 0; j < 8; j++) {
            int bit = (extracted_watermark[i] >> (7 - j)) & 1;
            c |= (bit << (7 - j));
        }
        extracted_string[i] = c;
    }
    
    printf("Extracted from noisy image: \"%s\"\n", extracted_string);
    similarity = calculate_similarity(watermark, extracted_watermark, watermark_length);
    printf("Similarity after noise: %.2f%%\n", similarity * 100);
    
    // Test with JPEG compression artifacts
    printf("\nTesting robustness with JPEG compression...\n");
    
    // Save with lower quality and reload
    save_jpeg(watermarked, "temp_low_quality.jpg", 50);
    Image *jpeg_compressed = load_jpeg("temp_low_quality.jpg");
    
    if (jpeg_compressed) {
        save_jpeg(jpeg_compressed, "jpeg_compressed_watermarked.jpg", 90);
        
        // Extract watermark from JPEG compressed image
        memset(extracted_watermark, 0, sizeof(extracted_watermark));
        extract_watermark(jpeg_compressed, extracted_watermark, watermark_length);
        
        // Convert to string
        memset(extracted_string, 0, sizeof(extracted_string));
        for (int i = 0; i < strlen(watermark); i++) {
            char c = 0;
            for (int j = 0; j < 8; j++) {
                int bit = (extracted_watermark[i] >> (7 - j)) & 1;
                c |= (bit << (7 - j));
            }
            extracted_string[i] = c;
        }
        
        printf("Extracted from JPEG compressed image: \"%s\"\n", extracted_string);
        similarity = calculate_similarity(watermark, extracted_watermark, watermark_length);
        printf("Similarity after JPEG compression: %.2f%%\n", similarity * 100);
        
        free_image(jpeg_compressed);
    }
    
    // Remove temporary file
    remove("temp_low_quality.jpg");
    
    printf("\nGenerated files:\n");
    printf("- original_test_image.jpg (original test image)\n");
    printf("- watermarked_image.jpg (image with embedded watermark)\n");
    printf("- noisy_watermarked_image.jpg (watermarked image with noise)\n");
    printf("- jpeg_compressed_watermarked.jpg (watermarked image after JPEG compression)\n");
    
    // Cleanup
    free_image(original);
    free_image(watermarked);
    free_image(noisy);
    
    printf("\nDemo completed successfully!\n");
    return 0;
}