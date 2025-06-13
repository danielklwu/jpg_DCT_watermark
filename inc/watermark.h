#ifndef WATERMARK_H
#define WATERMARK_H

#include "image.h"

// Watermarking functions
void embed_watermark(Image *img, char *watermark, int watermark_length, double alpha);
void extract_watermark(Image *img, char *extracted_watermark, int watermark_length);
double calculate_similarity(char *watermark1, char *watermark2, int length);
void generate_sequence(int *sequence, int length, int seed);

#endif
