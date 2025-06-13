#include <stdlib.h>
#include <stdio.h>
#include "attacks.h"

Image* attack_noise(Image* img, int noise_level) {
    Image* noisy = copy_image(img);
    
    srand(54321); // Fixed seed for reproducibility
    for (int i = 0; i < noisy->height; i++) {
        for (int j = 0; j < noisy->width; j++) {
            int noise = (rand() % (2 * noise_level + 1)) - noise_level;
            int new_val = (int)noisy->data[i][j] + noise;
            if (new_val < 0) new_val = 0;
            if (new_val > 255) new_val = 255;
            noisy->data[i][j] = (unsigned char)new_val;
        }
    }
    
    return noisy;
}

Image* attack_quality(Image* img, int quality) {
    char temp_filename[] = "temp_low_quality.jpg";
    
    // Save with lower quality
    if (!save_jpeg(img, temp_filename, quality)) {
        return NULL;
    }
    
    // Reload the compressed image
    Image* compressed = load_jpeg(temp_filename);
    
    // Clean up temporary file
    remove(temp_filename);
    
    return compressed;
}
