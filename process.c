#include "stdio.h"
#include "process.h"
#include "math.h"

// Local helper function declaration
void sawtooth_wave(uint8_t *wave_table, uint16_t size);
void sin_wave(uint8_t *wave_table, uint16_t size);

uint8_t cnt = 0;

void process_init(){
}

void process(const uint16_t *inp, uint8_t *outp, uint16_t size) {
    for(uint16_t idx = 0; idx < size; idx++ ) {
        outp[idx] = (inp[idx]>>3);
    }
}

//
// Some helper functions testing the 5bits R-2R DAC
//

// Set up wavetable: 5 bits sawtooth 
void sawtooth_wave(uint8_t *wave_table, uint16_t size) {
    for(uint16_t i = 0; i<size; i++ ) {
        wave_table[i] = (uint8_t)(i>>3);
    }
}

// Set up wavetable: 5 bits sinus
void sin_wave(uint8_t *wave_table, uint16_t size) {
    for (int i = 0; i < size; i++) {
        float phase = 2.0f * M_PI * i / size;
        float s = (sinf(phase) + 1.0f) * 127.5f;  // 0..255
        wave_table[i] = (uint8_t)s>>3;
    }
}

