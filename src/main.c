#include "main.h"

// Prints original image with no changes (debug)
#define PRINT_ORIG 0

// Function to clean up DCT data
void cleanup_dct_data(dct_data_t *dct_data) {
    if (dct_data) {
        if (dct_data->cinfo) {
            jpeg_finish_decompress(dct_data->cinfo);
            jpeg_destroy_decompress(dct_data->cinfo);
            free(dct_data->cinfo);
        }
        free(dct_data);
    }
}

// Main watermarking function
int watermark_jpeg(const char* input_file, const char* output_file, 
                  const char* watermark) {
    printf("Starting JPEG watermarking process...\n");
    printf("Input: %s\n", input_file);
    printf("Output: %s\n", output_file);
    
    // Read JPEG and get DCT coefficients
    dct_data_t *dct_data = read_jpeg_dct(input_file);
    if (!dct_data) {
        fprintf(stderr, "Failed to read DCT coefficients from %s\n", input_file);
        return 0;
    }
    
    printf("Successfully read DCT coefficients\n");
    printf("Image dimensions: %dx%d\n", 
           dct_data->cinfo->image_width, dct_data->cinfo->image_height);
    printf("Number of components: %d\n", dct_data->cinfo->num_components);

#if PRINT_ORIG
    write_jpeg_dct(dct_data, "orig.jpeg");
#endif
    
    // Embed watermark
    int bits_embedded = embed_watermark(dct_data->cinfo, dct_data->coef_arrays, watermark);
    if (bits_embedded == 0) {
        fprintf(stderr, "Failed to embed watermark\n");
        cleanup_dct_data(dct_data);
        return 0;
    }
    
    // Write watermarked JPEG
    if (!write_jpeg_dct(dct_data, output_file)) {
        fprintf(stderr, "Failed to write output file %s\n", output_file);
        cleanup_dct_data(dct_data);
        return 0;
    }
    
    printf("Successfully wrote watermarked image to %s\n", output_file);
    
    // Cleanup
    cleanup_dct_data(dct_data);
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <input.jpg> <output.jpg> <watermark_text>\n", argv[0]);
        printf("Example: %s image.jpg watermarked.jpg \"Copyright 2025\"\n", argv[0]);
        return 1;
    }
    
    const char *input_file = argv[1];
    const char *output_file = argv[2];
    const char *watermark = argv[3];
    
    if (watermark_jpeg(input_file, output_file, watermark)) {
        printf("Watermarking completed successfully!\n");
        return 0;
    } else {
        printf("Watermarking failed!\n");
        return 1;
    }
}