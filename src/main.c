#include "main.h"
#include "convert.h"

#define ENCODE 1
#define EXTRACT 1
#define TEST_ATTACKS 1

int main(int argc, char *argv[]) {
    // printf("JPEG library version: %d\n", JPEG_LIB_VERSION);

    if (argc != 2) {
        printf("Usage: %s <input_image>\n", argv[0]);
        printf("Example: %s input1.jpg\n", argv[0]);
        return 1;
    }

    printf("DCT Watermarking Algorithm Demo\n");
    printf("=========================================\n\n");
    
    // Initialize DCT tables
    init_dct_tables();
    
    // Load input image
    char* filename = argv[1];
    // Detect input format
    const char* ext = strrchr(filename, '.');
    int is_jpg = 0;
    char temp_jpeg[] = "__temp_input.jpg";
    char temp_out_jpeg[] = "__temp_output.jpg";
    char output_file[256];
    char reconverted_file[256];
    if (ext && (strcasecmp(ext+1, "jpg") == 0 || strcasecmp(ext+1, "jpeg") == 0)) {
        is_jpg = 1;
    }

    MyImage *original = NULL;
    if (is_jpg) {
        original = load_jpeg(filename);
    } else {
        // Convert to JPEG for processing
        original = convert_to_jpeg(filename, temp_jpeg);
        if (!original) {
            printf("Error: Could not convert %s to JPEG.\n", filename);
            return 1;
        }
    }
    if (!original) {
        printf("Error: Failed to load %s\n", filename);
        return 1;
    }
    printf("Loaded input image: %dx%d pixels\n", original->width, original->height);

    /* Create test image
    MyImage *original = create_image(256, 256);
    create_test_image(original);
    printf("Created test image: %dx%d pixels\n", original->width, original->height);
    save_jpeg(original, "original_test_image.jpg", 90);
    */
    
    // Create a copy for watermarking
    MyImage *watermarked = copy_image(original);
    
    // Create watermark
    char watermark[] = "WATERMARK_TEST_123";
    int watermark_length = strlen(watermark) * 8; // Convert to bits
    printf("Original watermark: \"%s\" (%d bits)\n", watermark, watermark_length);
    printf("Watermark in bits: ");
    for (int i = 0; i < strlen(watermark); i++) {
        for (int j = 7; j >= 0; j--) {
            printf("%d", (watermark[i] >> j) & 1);
        }
    }
    printf("\n");
    
#if ENCODE    
    // Embed watermark
    double alpha = 50.0; // Embedding strength
    printf("Embedding watermark with strength alpha = %.1f\n", alpha);
    embed_watermark(watermarked, watermark, watermark_length, alpha);
    printf("Watermark embedded successfully!\n");
    
    // Save watermarked image
    if (is_jpg) {
        save_jpeg(watermarked, "watermarked_image.jpg", 90);
        strcpy(output_file, "watermarked_image.jpg");
    } else {
        save_jpeg(watermarked, temp_out_jpeg, 90);
        // Reconvert to original format
        snprintf(reconverted_file, sizeof(reconverted_file), "watermarked_image%s", ext);
        if (!convert_from_jpeg(temp_out_jpeg, reconverted_file, ext+1)) {
            printf("Error: Could not convert output JPEG to original format.\n");
        } else {
            printf("Output reconverted to original format: %s\n", reconverted_file);
        }
        strcpy(output_file, reconverted_file);
    }
#endif

#if EXTRACT
    
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
#endif

#if TEST_ATTACKS
    
    // Test robustness with noise attack
    printf("\nTesting robustness with noise...\n");
    MyImage *noisy = attack_noise(watermarked, 10);
    
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
    
    // Apply quality attack
    MyImage *jpeg_compressed = attack_quality(watermarked, 50);
    
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
    
    free_image(noisy);
#endif
    
    printf("\nGenerated files:\n");
    // printf("- original_test_image.jpg (original test image)\n");
    printf("- watermarked_image.jpg (image with embedded watermark)\n");
#if TEST_ATTACKS
    printf("- noisy_watermarked_image.jpg (watermarked image with noise)\n");
    printf("- jpeg_compressed_watermarked.jpg (watermarked image after JPEG compression)\n");
#endif
    
    // Cleanup
    free_image(original);
    free_image(watermarked);
    
    // Cleanup temp files
    if (!is_jpg) {
        remove(temp_jpeg);
        remove(temp_out_jpeg);
    }
    
    printf("\n=========================================\n");
    return 0;
}
