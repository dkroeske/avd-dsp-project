#include "stdio.h"
#include "process.h"
#include "math.h"

// Local helper function declaration
void sawtooth_wave(uint8_t *wave_table, uint16_t size);
void sin_wave(uint8_t *wave_table, uint16_t size);
void dac2float(const float *src, uint8_t *dest, uint16_t size, float scaling);
void adc2float(const uint16_t *src, float *dest, uint16_t size, float scaling);


/* ************************************************************************** */
void dac2float(const float *src, uint8_t *dest, uint16_t size, float scaling)
/* 
short   :
onputs  :
outputs :
notes   : Convert to DAC (32 bits)
version : DMK. Intial code
***************************************************************************** */
{
    for(uint16_t idx = 0; idx < size; idx++) {
        dest[idx] = (uint8_t) (src[idx] * scaling);
    }
}

/* ************************************************************************** */
void adc2float(const uint16_t *src, float *dest, uint16_t size, float scaling)
/* 
short   :
onputs  :
outputs :
notes   : Convert from ADC to float
version : DMK. Intial code
***************************************************************************** */
{
    for(uint16_t idx = 0; idx < size; idx++) {
        dest[idx] = (float) (src[idx] / scaling);
    }
}

/* ************************************************************************** */
void process_init()
/* 
short   :
inputs  :
outputs :
notes   : 
version : DMK. Intial code
***************************************************************************** */
{
}

/* ************************************************************************** */
void process(const uint16_t *inp, uint8_t *outp, uint16_t size) 
/* 
short   :
inputs  :
outputs :
notes   : 
version : DMK. Intial code
***************************************************************************** */
{
    for(uint16_t idx = 0; idx < size; idx++ ) {
        outp[idx] = (inp[idx]>>3);
    }
}





/* *****************************************************************************
 Some helper functions testing the 5bits R-2R DAC

***************************************************************************** */

/* ************************************************************************** */
void sawtooth_wave(uint8_t *wave_table, uint16_t size) 
/* 
short   :
inputs  :
outputs :
notes   : Fills a block with a triangle
version : DMK. Intial code
***************************************************************************** */
{
    for(uint16_t i = 0; i<size; i++ ) {
        wave_table[i] = (uint8_t)(i>>3);
    }
}

/* ************************************************************************** */
void sin_wave(uint8_t *wave_table, uint16_t size) 
/* 
short   :
inputs  :
outputs :
notes   : Fills a block with a sin wave
version : DMK. Intial code
***************************************************************************** */
{
    for (int i = 0; i < size; i++) {
        float phase = 2.0f * M_PI * i / size;
        float s = (sinf(phase) + 1.0f) * 127.5f;  // 0..255
        wave_table[i] = (uint8_t)s>>3;
    }
}


