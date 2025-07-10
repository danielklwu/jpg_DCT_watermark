#include <math.h>
#include "dct.h"

// Global DCT coefficient matrices
double dct_coeff[BLOCK_SIZE][BLOCK_SIZE];
double idct_coeff[BLOCK_SIZE][BLOCK_SIZE];

void init_dct_tables() {
    int i, j;
    double alpha_i, alpha_j;
    
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            alpha_i = (i == 0) ? sqrt(1.0/BLOCK_SIZE) : sqrt(2.0/BLOCK_SIZE);
            alpha_j = (j == 0) ? sqrt(1.0/BLOCK_SIZE) : sqrt(2.0/BLOCK_SIZE);
            
            dct_coeff[i][j] = alpha_i * alpha_j * 
                cos((2*i + 1) * PI * j / (2.0 * BLOCK_SIZE)) *
                cos((2*j + 1) * PI * i / (2.0 * BLOCK_SIZE));
            
            idct_coeff[i][j] = dct_coeff[i][j];
        }
    }
}

void forward_dct(double input[BLOCK_SIZE][BLOCK_SIZE], double output[BLOCK_SIZE][BLOCK_SIZE]) {
    int u, v, i, j;
    double sum;
    double alpha_u, alpha_v;
    
    for (u = 0; u < BLOCK_SIZE; u++) {
        for (v = 0; v < BLOCK_SIZE; v++) {
            sum = 0.0;
            alpha_u = (u == 0) ? sqrt(1.0/BLOCK_SIZE) : sqrt(2.0/BLOCK_SIZE);
            alpha_v = (v == 0) ? sqrt(1.0/BLOCK_SIZE) : sqrt(2.0/BLOCK_SIZE);
            
            for (i = 0; i < BLOCK_SIZE; i++) {
                for (j = 0; j < BLOCK_SIZE; j++) {
                    sum += input[i][j] * 
                           cos((2*i + 1) * PI * u / (2.0 * BLOCK_SIZE)) *
                           cos((2*j + 1) * PI * v / (2.0 * BLOCK_SIZE));
                }
            }
            output[u][v] = alpha_u * alpha_v * sum;
        }
    }
}

void inverse_dct(double input[BLOCK_SIZE][BLOCK_SIZE], double output[BLOCK_SIZE][BLOCK_SIZE]) {
    int i, j, u, v;
    double sum;
    double alpha_u, alpha_v;
    
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            sum = 0.0;
            
            for (u = 0; u < BLOCK_SIZE; u++) {
                for (v = 0; v < BLOCK_SIZE; v++) {
                    alpha_u = (u == 0) ? sqrt(1.0/BLOCK_SIZE) : sqrt(2.0/BLOCK_SIZE);
                    alpha_v = (v == 0) ? sqrt(1.0/BLOCK_SIZE) : sqrt(2.0/BLOCK_SIZE);
                    
                    sum += alpha_u * alpha_v * input[u][v] *
                           cos((2*i + 1) * PI * u / (2.0 * BLOCK_SIZE)) *
                           cos((2*j + 1) * PI * v / (2.0 * BLOCK_SIZE));
                }
            }
            output[i][j] = sum;
        }
    }
}
