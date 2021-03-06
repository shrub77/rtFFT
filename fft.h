#ifndef FFT_H_INCLUDED
#define FFT_H_INCLUDED

#include <complex.h>
#include <stddef.h>
#include <limits.h>
#include <math.h>
#include "error.h"
#define INTSIZE sizeof(uint_t) * CHAR_BIT

typedef unsigned int uint_t;
typedef double complex dcomp_t;
static const int MultiplyDeBruijnBitPosition2[32] = 
{
      0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
        31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};

//Checks is n is a power of 2
static int checkPowerTwo(uint_t n);

//From bit twiddiling hacks
static uint_t reverseBits(uint_t n);

//Assumes n is a power of two
static uint_t fastlog2(uint_t n);

//store's the i-th value in input into the bit-reversed-i-th index in output
static int bitReverseCopy(dcomp_t input[], dcomp_t output[], size_t length);

//Cooley-Turkey Algorithm implementation
static int iterativeFFT(dcomp_t input[], dcomp_t output[], size_t length);


int checkPowerTwo(uint_t n)
{
    if(n > UINT_MAX){
        return E_INVALID_INPUT;
    }
    return ((n & (n -1)) == 0);
}


uint_t fastlog2(uint_t n)
{
    return MultiplyDeBruijnBitPosition2[(uint_t)(n * 0x077CB531U) >> 27];
}

uint_t reverseBits(uint_t num)
{
    num = ((num >> 1) & 0x55555555) | ((num & 0x55555555) << 1);
    // swap consecutive pairs
    num = ((num >> 2) & 0x33333333) | ((num & 0x33333333) << 2);
    // swap nibbles ... 
    num = ((num >> 4) & 0x0F0F0F0F) | ((num & 0x0F0F0F0F) << 4);
    // swap bytes
    num = ((num >> 8) & 0x00FF00FF) | ((num & 0x00FF00FF) << 8);
    // swap 2-byte long pairs
    num = ( num >> 16             ) | ( num               << 16);
    return num;
}

int bitReverseCopy(dcomp_t input[], dcomp_t output[], size_t length)
{
    if(checkPowerTwo(length) == E_INVALID_INPUT || checkPowerTwo(length) == 0)
    {
        eprintf(E_INVALID_INPUT, "Given array length is not a power of 2");
        return E_INVALID_INPUT;
    }
    uint_t wordlength = fastlog2(length);
    for(uint_t i = 0; i < length; i++)
    {
        output[reverseBits(i) >> (INTSIZE - wordlength)] = input[i];
    }
    return E_SUCCESS;
}

int iterativeFFT(dcomp_t input[], dcomp_t output[], size_t length)
{
    if(bitReverseCopy(input, output, length) != E_SUCCESS)
    {
        return E_INVALID_INPUT;
    }
    for(uint_t i = 1; i <= fastlog2(length); i++)
    {
        uint_t m = 1 << i;
        dcomp_t omega_m = cexp(-2*M_PI*I/m);

        for(uint_t j = 0; j < length; j += m)
        {
            dcomp_t omega = 1.0;
            for(uint_t k = 0; k < (m/2); k++)
            {
                dcomp_t t = omega*output[j + k + m/2];
                dcomp_t u = output[j + k];
                output[j + k] = u + t;
                output[j + k + m/2] = u - t;
                omega = omega * omega_m;            
            }
        }
    }
    return E_SUCCESS;
}


#endif
