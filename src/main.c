#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "dct.h"
#include "watermark.h"
#include "attacks.h"

#define TEST_ATTACKS 1

int main() {
    printf("DCT Watermarking Algorithm Demo\n");
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


#ifdef TEST_ATTACKS
    
    // Test robustness with noise attack
    printf("\nTesting robustness with noise...\n");
    Image *noisy = attack_noise(watermarked, 10);
    
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
    Image *jpeg_compressed = attack_quality(watermarked, 50);
    
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
#endif
    
    printf("\nGenerated files:\n");
    printf("- original_test_image.jpg (original test image)\n");
    printf("- watermarked_image.jpg (image with embedded watermark)\n");
    printf("- noisy_watermarked_image.jpg (watermarked image with noise)\n");
    printf("- jpeg_compressed_watermarked.jpg (watermarked image after JPEG compression)\n");
    
    // Cleanup
    free_image(original);
    free_image(watermarked);
    free_image(noisy);
    
    printf("\n=========================================\n");
    return 0;
}
