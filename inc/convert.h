#ifndef CONVERT_H
#define CONVERT_H

#include "image.h"

// Convert any image format to JPEG for processing
MyImage* convert_to_jpeg(const char* input_path, const char* temp_jpeg_path);

// Convert JPEG back to original format
int convert_from_jpeg(const char* jpeg_path, const char* output_path, const char* original_format);

#endif
