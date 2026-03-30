#ifndef CONV_DIRECT_INCLUDE
#define CONV_DIRECT_INCLUDE

typedef struct fir_t fir_t;
typedef fir_t *fir_handle_t;

fir_handle_t fir_init(const float *coeffs, uint16_t coeffs_size, uint16_t block_size, uint16_t gain );
void fir_update(fir_handle_t f, float *inp, float *outp, uint16_t block_size);

/* Syncfilter bandpass filter zoals in de les gemaakt */
#define BPF800_1200_CONVDIRECT_KERNEL_GAIN      1.00f
#define BPF800_1200_KERNEL_LENGHT    100 
extern const float bpf800_1200_kernel[];

#endif
