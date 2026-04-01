#ifndef CONV_DIRECT_INCLUDE
#define CONV_DIRECT_INCLUDE

typedef struct fir_t fir_t;
typedef fir_t *fir_handle_t;

fir_handle_t fir_init(const float *coeffs, uint16_t num_taps, uint16_t block_size, uint16_t gain );
void fir_update(fir_handle_t f, float *inp, float *outp, uint16_t block_size);

#define ISF_KERNEL_LENGHT   21
extern const float isf_kernel[];

#endif
