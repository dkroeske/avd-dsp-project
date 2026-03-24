#ifndef BANDPASS_INCLUDE
#define BANDPASS_INCLUDE

typedef struct fir_t fir_t;
typedef fir_t *fir_handle_t;

fir_handle_t fir_init(const float *coeffs, uint16_t coeffs_size, uint16_t block_size, uint16_t gain );
void fir_update(fir_handle_t f, float *inp, float *outp, uint16_t block_size);

/* Syncfilter zoals in de les gemaakt */
#define CONVDIRECT_KERNEL_GAIN      1.00f
#define CONVDIRECT_KERNEL_LENGHT    100 
extern const float ConvDirect_Kernel[];

#endif
