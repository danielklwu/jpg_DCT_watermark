#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>
#include "image.h"

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

void free_image(Image *img) {
    for (int i = 0; i < img->height; i++) {
        free(img->data[i]);
    }
    free(img->data);
    free(img);
}

Image* copy_image(Image *src) {
    Image *dst = create_image(src->width, src->height);
    for (int i = 0; i < src->height; i++) {
        memcpy(dst->data[i], src->data[i], src->width);
    }
    return dst;
}

void add_noise(Image *img, int noise_level) {
    srand(54321);
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

void create_test_image(Image *img) {
    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            img->data[i][j] = (unsigned char)((i + j) % 256);
        }
    }
}

// JPEG functions implementation
int save_jpeg(Image *img, const char *filename, int quality) {
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *outfile;
    JSAMPROW row_pointer[1];
    
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    
    if ((outfile = fopen(filename, "wb")) == NULL) {
        printf("Error: Cannot create JPEG file %s\n", filename);
        jpeg_destroy_compress(&cinfo);
        return 0;
    }
    
    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width = img->width;
    cinfo.image_height = img->height;
    cinfo.input_components = 1;
    cinfo.in_color_space = JCS_GRAYSCALE;
    
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);
    
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &img->data[cinfo.next_scanline][0];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }
    
    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);
    
    printf("Saved JPEG image as %s (quality: %d)\n", filename, quality);
    return 1;
}

Image* load_jpeg(const char *filename) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *infile;
    JSAMPARRAY buffer;
    Image *img;
    
    if ((infile = fopen(filename, "rb")) == NULL) {
        printf("Error: Cannot open JPEG file %s\n", filename);
        return NULL;
    }
    
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    
    if (cinfo.jpeg_color_space != JCS_GRAYSCALE) {
        cinfo.out_color_space = JCS_GRAYSCALE;
    }
    
    jpeg_start_decompress(&cinfo);
    img = create_image(cinfo.output_width, cinfo.output_height);
    
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, 
                                       cinfo.output_width * cinfo.output_components, 1);
    
    int row = 0;
    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        memcpy(img->data[row], buffer[0], cinfo.output_width);
        row++;
    }
    
    jpeg_finish_decompress(&cinfo);
    fclose(infile);
    jpeg_destroy_decompress(&cinfo);
    
    printf("Loaded JPEG image from %s (%dx%d)\n", filename, img->width, img->height);
    return img;
}
