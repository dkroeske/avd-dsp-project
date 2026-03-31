/**************************************************

Avans DSP project on Pico 1 or 2 microcontroller
 
    (c) dkroeske@gmail.com

    v1.0    03/19/2026 Initial code

***************************************************/

// Global includes
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "hardware/adc.h"
#include "hardware/pio.h"

// Local includes
#include "process.h"
#include "dac_r2r.pio.h"

// Constants, macro's, ...
#define ADC_PIN         26
#define ADC_FS          20000.0f    // Sample frequency ADC
#define DAC_FS          20000.0f    // Update frequenct DAC
#define BLOCK_SIZE      1024        // Sample buffer size

// Global vars
int dma_adc_ping_channel;         
int dma_adc_pong_channel;
dma_channel_config adc_ping_config;
dma_channel_config adc_pong_config;

int dma_dac_ping_channel;
int dma_dac_pong_channel;
dma_channel_config dac_ping_config;
dma_channel_config dac_pong_config;

// ADC input buffers
uint16_t ping[BLOCK_SIZE];
uint16_t pong[BLOCK_SIZE];

// DAC output buffers
uint8_t dac_ping[BLOCK_SIZE];
uint8_t dac_pong[BLOCK_SIZE];

/* ************************************************************************** */
static void __isr __time_critical_func(dma_handler)()
/* 
short   :
inputs  :
outputs :
notes   : Interrupt Service Routine (ISR) for dma controller. Both AD
          channel will trigger the IST
version : DMK. Intial code
***************************************************************************** */
{
    if( dma_hw->ints0 & (1u << dma_adc_ping_channel) ) {

        // adc
        dma_channel_set_write_addr(dma_adc_ping_channel, ping, false);
        // dac
        dma_channel_set_read_addr(dma_dac_pong_channel, dac_pong, true);
        // restart isr_adc
        dma_hw->ints0 = 1u << dma_adc_ping_channel;
    }
    
    if( dma_hw->ints0 & (1u << dma_adc_pong_channel) ) {

        // adc
        dma_channel_set_write_addr(dma_adc_pong_channel, pong, false);
        // dac
        dma_channel_set_read_addr(dma_dac_ping_channel, dac_ping, true);
        // restart isr_adc
        dma_hw->ints0 = 1u << dma_adc_pong_channel;
    }
}


/* ************************************************************************** */
int main() 
/* 
short   :
inputs  :
outputs :
notes   : Mainloop, entry point after reset. Lot's of initialisation ADC, DMA
          channel, PIO DAC (R-2R network).
version : DMK. Intial code
***************************************************************************** */
{
    stdio_init_all();
    printf("\n---------------------------------------\n");
    printf("DSP Project Rapsberry Pico 2040 or 2350\n\n");

    // Inform world about clockspeeds
    uint32_t f_sys_clk = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
    printf("System clock [khz]: %ld\n", f_sys_clk);
    uint32_t f_adc_clk = clock_get_hz(clk_adc);
    printf("ADC clock [hz]    : %ld\n", f_adc_clk);

    // ADC Init
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_set_clkdiv(f_adc_clk / ADC_FS); 
    hw_clear_bits(&adc_hw->fcs, ADC_FCS_UNDER_BITS);
    hw_clear_bits(&adc_hw->fcs, ADC_FCS_OVER_BITS);
    adc_fifo_setup(
        true,
        true,
        1,
        false,
        true
    );
    adc_select_input(0);

    // Init 2 DMA channels and chain them together. Link to ADC
    dma_adc_ping_channel = dma_claim_unused_channel(true);
    dma_adc_pong_channel = dma_claim_unused_channel(true);
    adc_ping_config = dma_channel_get_default_config(dma_adc_ping_channel);
    adc_pong_config = dma_channel_get_default_config(dma_adc_pong_channel);

    channel_config_set_transfer_data_size(&adc_ping_config, DMA_SIZE_16);
    channel_config_set_read_increment(&adc_ping_config, false);
    channel_config_set_write_increment(&adc_ping_config, true);
    channel_config_set_dreq(&adc_ping_config, DREQ_ADC);
    channel_config_set_chain_to(&adc_ping_config, dma_adc_pong_channel);
    dma_channel_configure(
        dma_adc_ping_channel,
        &adc_ping_config,
        ping,
        &adc_hw->fifo,
        BLOCK_SIZE,
        false 
    );
    dma_channel_set_irq0_enabled(dma_adc_ping_channel, true);
    
    channel_config_set_transfer_data_size(&adc_pong_config, DMA_SIZE_16);
    channel_config_set_read_increment(&adc_pong_config, false);
    channel_config_set_write_increment(&adc_pong_config, true);
    channel_config_set_dreq(&adc_pong_config, DREQ_ADC);
    channel_config_set_chain_to(&adc_pong_config, dma_adc_ping_channel);
    dma_channel_configure(
        dma_adc_pong_channel,
        &adc_pong_config,
        pong,
        &adc_hw->fifo,
        BLOCK_SIZE,
        false 
    );
    dma_channel_set_irq0_enabled(dma_adc_pong_channel, true);
    
    // ADC IRQ's
    dma_set_irq0_channel_mask_enabled((1u<<dma_adc_ping_channel) | (1u << dma_adc_pong_channel), true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    
    // DAC, init R-2R network
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &dac_r2r_program);
    int sm = pio_claim_unused_sm(pio, true);
    dac_r2r_program_init(pio, sm, offset, 8, DAC_FS);

    // DMA channels, link to DAC
    dma_dac_ping_channel = dma_claim_unused_channel(true);
    dac_ping_config = dma_channel_get_default_config(dma_dac_ping_channel);
    channel_config_set_transfer_data_size(&dac_ping_config, DMA_SIZE_8);
    channel_config_set_read_increment(&dac_ping_config, true);
    channel_config_set_write_increment(&dac_ping_config, false);
    channel_config_set_dreq(&dac_ping_config, pio_get_dreq(pio, sm, true));
    dma_channel_configure(
        dma_dac_ping_channel,
        &dac_ping_config,
        &pio->txf[sm],
        dac_ping,
        BLOCK_SIZE,
        false
    );
    
    dma_dac_pong_channel = dma_claim_unused_channel(true);
    dac_pong_config = dma_channel_get_default_config(dma_dac_pong_channel);
    channel_config_set_transfer_data_size(&dac_pong_config, DMA_SIZE_8);
    channel_config_set_read_increment(&dac_pong_config, true);
    channel_config_set_write_increment(&dac_pong_config, false);
    channel_config_set_dreq(&dac_pong_config, pio_get_dreq(pio, sm, true));
    dma_channel_configure(
        dma_dac_pong_channel,
        &dac_pong_config,
        &pio->txf[sm],
        dac_pong,
        BLOCK_SIZE,
        false
    );

    // Let's rock & roll => start ADC
    dma_start_channel_mask(1u << dma_adc_ping_channel);
    adc_run(true);

    // Init fir or filter
    process_init(); 

    printf("Let's go ...\n");

    while (true) {
    
        dma_channel_wait_for_finish_blocking(dma_adc_ping_channel);
        process(ping, dac_ping, BLOCK_SIZE);
//        printf("ping ...\n");
 
        dma_channel_wait_for_finish_blocking(dma_adc_pong_channel);
        process(pong, dac_pong, BLOCK_SIZE);
//        printf("\tpong ...\n");

    }
}

