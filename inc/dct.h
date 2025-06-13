#ifndef DCT_H
#define DCT_H

#define BLOCK_SIZE 8
#define PI 3.14159265359

// DCT functions
void init_dct_tables(void);
void forward_dct(double input[BLOCK_SIZE][BLOCK_SIZE], double output[BLOCK_SIZE][BLOCK_SIZE]);
void inverse_dct(double input[BLOCK_SIZE][BLOCK_SIZE], double output[BLOCK_SIZE][BLOCK_SIZE]);

#endif
