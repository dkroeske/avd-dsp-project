#include "stdio.h"
#include "process.h"
#include "conv_direct.h"
#include "math.h"


// Local helper function declaration
void sawtooth_wave(uint8_t *wave_table, uint16_t size);
void sin_wave(uint8_t *wave_table, uint16_t size);
void float2dac(const float *src, uint8_t *dest, uint16_t size, float scaling);
void adc2float(const uint16_t *src, float *dest, uint16_t size, float scaling);

float out_f[BLOCK_SIZE]; // Resultaat direct convolutie
float in_f[BLOCK_SIZE];   // DAC values in float's
fir_handle_t isf;     // Inverse Sync Filter handle.

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
    // Init Inverse Sync or DAC reconstruction filter
    isf = fir_init( isf_kernel, ISF_KERNEL_LENGHT, BLOCK_SIZE, 1.0);
    if( isf == NULL ) { 
        printf("ERROR: fir_init() failed, this is FATAL!!\n");
    } else {
        printf("process_init() ok\n");
    }
}

/* ************************************************************************** */
void process(const uint16_t *inp, uint8_t *outp) 
/* 
short   :
inputs  :
outputs :
notes   : 
version : DMK. Intial code
***************************************************************************** */
{
    // Convert all sampels naar float
    adc2float(inp, in_f, BLOCK_SIZE, 255.0f);
    
    // Met reconstructie filter
    // Do de convolutie (filter), in dit geval de reconstructie.
    fir_update(isf, in_f, out_f, BLOCK_SIZE);

    // Zonder reconstructie filter
    //for(uint16_t idx = 0; idx < BLOCK_SIZE; idx++ ) {
    //    out_f[idx] = in_f[idx];
   // }
    
    // ... result van float naar 5-bits outp voor DAC
    float2dac(out_f, outp, BLOCK_SIZE, 255.0f);
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


