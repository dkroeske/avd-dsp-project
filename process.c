#include "stdio.h"
#include "process.h"
#include "math.h"

// Local helper function declaration
void sawtooth_wave(uint8_t *wave_table, uint16_t size);
void sin_wave(uint8_t *wave_table, uint16_t size);
void float2dac(const float *src, uint8_t *dest, uint16_t size, float scaling);
void adc2float(const uint16_t *src, float *dest, uint16_t size, float scaling);

float fbuf[1024];
uint8_t wave_table[1024];


/* ************************************************************************** */
void float2dac(const float *src, uint8_t *dest, uint16_t size, float scaling)
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
        dest[idx] >>= 1;    // Adjust for 7 bits DAC
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
    sin_wave(wave_table, 1024); 
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
    adc2float(inp, fbuf, 1024, 255.0f);
    float2dac(fbuf, outp, 1024, 255.0f);

//    for(uint16_t idx = 0; idx < size; idx++ ) {
//        outp[idx] = wave_table[idx]>>1; // Adjust for 7 bits dac
//    }
}


/* *****************************************************************************
 Some helper functions testing the 7-bits R-2R DAC
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
        wave_table[i] = (uint8_t)((1.0f * i)/size);
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
        float s = 0.8f*(sinf(phase) + 1.0f) * 127.5f;  // 0..255
        wave_table[i] = (uint8_t)s;
    }
}


