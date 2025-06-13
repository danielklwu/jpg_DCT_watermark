#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

typedef struct {
    unsigned char **data;
    int width;
    int height;
} Image;

// Image manipulation functions
Image* create_image(int width, int height);
void free_image(Image *img);
Image* copy_image(Image *src);
void add_noise(Image *img, int noise_level);
void create_test_image(Image *img);

// JPEG operations
int save_jpeg(Image *img, const char *filename, int quality);
Image* load_jpeg(const char *filename);

#endif
