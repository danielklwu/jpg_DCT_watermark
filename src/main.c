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
    if (argc < 3 || argc > 5) {
        printf("Usage:\n");
        printf("  Embed: %s embed <input.jpeg> <output.jpeg> <watermark_text>\n", argv[0]);
        printf("  Extract: %s extract <watermarked.jpeg>\n", argv[0]);
        printf("Example:\n");
        printf("  %s embed input.jpeg watermarked.jpeg \"copyright\"\n", argv[0]);
        printf("  %s extract watermarked.jpeg\n", argv[0]);
        return 1;
    }
    
    const char *operation = argv[1];
    
    if (!strcmp(operation, "embed")) {
        if (argc != 5) {
            printf("Embed usage: %s embed <input.jpeg> <output.jpeg> <watermark_text>\n", argv[0]);
            return 1;
        }
        
        const char *input_file = argv[2];
        const char *output_file = argv[3]; 
        const char *watermark = argv[4];
        
        if (watermark_jpeg(input_file, output_file, watermark)) {
            printf("Watermarking completed successfully!\n");
            return 0;
        } else {
            printf("Watermarking failed!\n");
            return 1;
        }
    }
    else if (!strcmp(operation, "extract")) {
        if (argc != 3) {
            printf("Extract usage: %s extract <watermarked.jpeg>\n", argv[0]);
            return 1;
        }
        
        const char *input_file = argv[2];
        char extracted_watermark[256];
        
        if (extract_watermark(input_file, extracted_watermark, 255)) {
            printf("Extracted watermark: '%s'\n", extracted_watermark);
            return 0;
        } else {
            printf("Failed to extract watermark!\n");
            return 1;
        }
    }
    else {
        printf("Unknown operation: %s\n", operation);
        printf("Use 'embed' or 'extract'\n");
        return 1;
    }
}