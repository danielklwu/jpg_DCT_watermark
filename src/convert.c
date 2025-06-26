#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ImageMagick-7/MagickWand/MagickWand.h>
#include "convert.h"
#include "image.h"

// Initialize ImageMagick environment
static void init_magick() {
    static int initialized = 0;
    if (!initialized) {
        MagickWandGenesis();
        initialized = 1;
    }
}

MyImage* convert_to_jpeg(const char* input_path, const char* temp_jpeg_path) {
    init_magick();
    MagickWand* magick_wand = NewMagickWand();
    MyImage* result = NULL;

    // Read the input image
    if (MagickReadImage(magick_wand, input_path) == MagickFalse) {
        printf("Error: Failed to read image %s\n", input_path);
        magick_wand = DestroyMagickWand(magick_wand);
        return NULL;
    }

    // Convert to grayscale
    if (MagickSetImageColorspace(magick_wand, GRAYColorspace) == MagickFalse) {
        printf("Error: Failed to convert to grayscale\n");
        magick_wand = DestroyMagickWand(magick_wand);
        return NULL;
    }

    // Set JPEG compression quality
    MagickSetImageCompressionQuality(magick_wand, 95);

    // Save as JPEG
    if (MagickWriteImage(magick_wand, temp_jpeg_path) == MagickFalse) {
        printf("Error: Failed to write JPEG image\n");
        magick_wand = DestroyMagickWand(magick_wand);
        return NULL;
    }

    // Clean up ImageMagick
    magick_wand = DestroyMagickWand(magick_wand);

    // Load the JPEG using our existing loader
    result = load_jpeg(temp_jpeg_path);

    return result;
}

int convert_from_jpeg(const char* jpeg_path, const char* output_path, const char* original_format) {
    init_magick();
    MagickWand* magick_wand = NewMagickWand();
    int success = 0;

    // Read the JPEG image
    if (MagickReadImage(magick_wand, jpeg_path) == MagickFalse) {
        printf("Error: Failed to read JPEG image\n");
        magick_wand = DestroyMagickWand(magick_wand);
        return 0;
    }

    // Set the output format
    if (MagickSetImageFormat(magick_wand, original_format) == MagickFalse) {
        printf("Error: Failed to set output format to %s\n", original_format);
        magick_wand = DestroyMagickWand(magick_wand);
        return 0;
    }

    // Write the output image
    if (MagickWriteImage(magick_wand, output_path) == MagickFalse) {
        printf("Error: Failed to write output image\n");
        magick_wand = DestroyMagickWand(magick_wand);
        return 0;
    }

    success = 1;
    magick_wand = DestroyMagickWand(magick_wand);
    return success;
}

// Cleanup function to be called at program exit
void cleanup_magick() {
    MagickWandTerminus();
}

