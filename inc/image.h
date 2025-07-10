#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

typedef struct {
    unsigned char **data;
    int width;
    int height;
} MyImage;

// Image manipulation functions
MyImage* create_image(int width, int height);
void free_image(MyImage *img);
MyImage* copy_image(MyImage *src);
void add_noise(MyImage *img, int noise_level);
void create_test_image(MyImage *img);

// JPEG operations
int save_jpeg(MyImage *img, const char *filename, int quality);
MyImage* load_jpeg(const char *filename);

#endif
