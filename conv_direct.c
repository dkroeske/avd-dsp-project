/* cmsis FIR */

#include "process.h"
#include "stdlib.h"
#include "arm_math.h"
#include "assert.h"
#include "conv_direct.h"

struct fir_t {
    arm_fir_instance_f32 instance;
    uint16_t numTaps;
    float *rpCoeffs;
    float *pState;
};

/*
 *
 */
fir_handle_t fir_init(const float *coeffs, uint16_t num_taps, uint16_t block_size, uint16_t gain ) {

    // New fir handler
    fir_handle_t fir = (fir_handle_t) malloc(sizeof(fir_t));
    if(!fir) {
        return NULL;
    }
   
    // Create and assign state variable array
    fir->pState = (float*) malloc((num_taps + block_size - 1) * sizeof(float));
    if(!fir->pState ) {
        free(fir);
        return NULL;
    }
    
    // 
    fir->numTaps = num_taps;

    // Reverse load fir coeffs
    fir->rpCoeffs = (float *) malloc(num_taps * sizeof(float)); 
    if(!fir->rpCoeffs) {
        free(fir->pState);
        free(fir);
        return 0;
    }

    for(uint16_t idx = 0; idx < fir->numTaps; idx++) {
        fir->rpCoeffs[idx] = gain * coeffs[num_taps - idx - 1];
    }

    // Init fir filter    
    arm_fir_init_f32(&fir->instance, num_taps, fir->rpCoeffs, fir->pState, block_size);

    return fir;
}

/*
 *
 */
void fir_update(fir_handle_t f,
 float *inp, float *outp, uint16_t block_size) {
    arm_fir_f32( &(f->instance), inp, outp, block_size);
}

// Inverse Sinc Filter (digital DAC reconstruction pre-compensation)
// fs = 40 kHz, zo vlak mogelijk tot circa 6 kHz, 21 taps
// 21 taps, DC gain = 1.0. Snelle probeersel in Scilab
const float isf_kernel[ISF_KERNEL_LENGHT] = {
    -0.00000000f,
    -0.00357278f,
    0.00403832f,
    0.00736675f,
    -0.02040857f,
    0.00000000f,
    0.05205059f,
    -0.05080639f,
    -0.08560116f,
    0.29648282f,
    0.60090084f,
    0.29648282f,
    -0.08560116f,
    -0.05080639f,
    0.05205059f,
    0.00000000f,
    -0.02040857f,
    0.00736675f,
    0.00403832f,
    -0.00357278f,
    -0.00000000f
};    


