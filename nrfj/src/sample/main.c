#include "uarte-commands.h"

// Serial communication 9600 baud

int main(){
    NRF_CLOCK->HFCLKCTRL = 0; // Black magic to start the HFCLK on the nRF5340. This is required for UART to work for some reason.
    init_uarte();
    nrf_gpio_cfg_output(36); // PIN_TXD_RPI (P1.04)
    nrf_gpio_pin_set(36);    // UART idle state is HIGH
    nrfx_systick_delay_ms(10); // Brief pause so the line settles high
    while (1) {
        print_string_over_GPIO("Hello World!\r\n");
        nrfx_systick_delay_ms(1000);
    }
    return 0;
}