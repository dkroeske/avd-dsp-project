//
// Avans dsp project on Raspberry pico 1 or 2
//

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "hardware/adc.h"
#include "hardware/pio.h"

#include "process.h"
#include "dac_r2r.pio.h"

// DAC test
uint32_t wave_table[256];    // 5 bits wave table

int dma_adc_ping_channel;
int dma_adc_pong_channel;
dma_channel_config adc_ping_config;
dma_channel_config adc_pong_config;

int dma_dac_ping_channel;
dma_channel_config dac_ping_config;

#define ADC_PIN         26
#define ADC_FS          20000.0f
#define DAC_FS          20000.0f
#define BLOCK_SIZE      256

// ADC input buffers
uint16_t ping[BLOCK_SIZE];
uint16_t pong[BLOCK_SIZE];

uint8_t dac_ping[BLOCK_SIZE];
uint8_t dac_pong[BLOCK_SIZE];

// ISR of ADC DMA
static void __isr __time_critical_func(dma_handler)() {

    if( dma_hw->ints0 & (1u << dma_adc_ping_channel) ) {

        // adc
        dma_channel_set_write_addr(dma_adc_ping_channel, ping, false);
        // dac
        dma_channel_set_read_addr(dma_dac_ping_channel, wave_table, true);
        // restart isr_adc
        dma_hw->ints0 = 1u << dma_adc_ping_channel;
    }
    
    if( dma_hw->ints0 & (1u << dma_adc_pong_channel) ) {

        // adc
        dma_channel_set_write_addr(dma_adc_pong_channel, pong, false);
        // dac
        dma_channel_set_read_addr(dma_dac_ping_channel, wave_table, true);
        // restart isr_adc
        dma_hw->ints0 = 1u << dma_adc_pong_channel;
    }
}


int main() {

    stdio_init_all();
    printf("DSP Project Rapsberry Pico 2040 or 2350\n");

    // Inform
    uint32_t f_sys_clk = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
    printf("System clock [khz] = %ld kHz\n", f_sys_clk);
    uint32_t f_adc_clk = clock_get_hz(clk_adc);
    printf("ADC clock [hz] = %ld Hz\n", f_adc_clk);

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
    
    // IRQ
    dma_set_irq0_channel_mask_enabled((1u<<dma_adc_ping_channel) | (1u << dma_adc_pong_channel), true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Init R-2 DAC
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &dac_r2r_program);
    int sm = pio_claim_unused_sm(pio, true);
    dac_r2r_program_init(pio, sm, offset, 8, DAC_FS);

    // Set up wavetable: 8 bits zaagtand
    //for(uint16_t idx = 0; idx<256; idx++ ) {
    //    wave_table[idx] = (uint32_t)(idx>>3);
    //}

    for (int i = 0; i < 256; i++) {
        float phase = 2.0f * M_PI * i / 256;
        float s = (sinf(phase) + 1.0f) * 127.5f;  // 0..255
        wave_table[i] = (uint32_t)((uint8_t)s>>3);
    }
    
    // DAC init
    dma_dac_ping_channel = dma_claim_unused_channel(true);
    dac_ping_config = dma_channel_get_default_config(dma_dac_ping_channel);
    channel_config_set_transfer_data_size(&dac_ping_config, DMA_SIZE_32);
    channel_config_set_read_increment(&dac_ping_config, true);
    channel_config_set_write_increment(&dac_ping_config, false);
    channel_config_set_dreq(&dac_ping_config, pio_get_dreq(pio, sm, true));
    dma_channel_configure(
        dma_dac_ping_channel,
        &dac_ping_config,
        &pio->txf[sm],
        wave_table,
        256,
        false
    );

    // Let's rock & roll
    dma_start_channel_mask(1u << dma_adc_ping_channel);
    adc_run(true);

    // Init fir filter
    process_init(); 

    printf("Let's go ...\n");


    while (true) {
    
/*
        for(uint16_t idx = 0; idx < 256; idx++ ) {
            pio_sm_put_blocking(pio, sm, (uint32_t)(wave_table[idx]>>3));
        //    printf("0x%02X\n", idx);
        //  sleep_ms(10);
        }
*/

        
/*
        pio_sm_put_blocking(pio, sm, 0x80>>3);
        printf("* ... ");
        sleep_ms(30);
        pio_sm_put_blocking(pio, sm, 0x00>>3);
        printf("+\n");
        sleep_ms(30);
*/



        dma_channel_wait_for_finish_blocking(dma_adc_ping_channel);
//        process(ping, dac_ping, BLOCK_SIZE);
//        printf("ping ...\n");
 
        dma_channel_wait_for_finish_blocking(dma_adc_pong_channel);
//        process(pong, dac_pong, BLOCK_SIZE);
//        printf("\tpong ...\n");

    }
}

