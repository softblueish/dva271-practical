#include "uarte-commands.h"

// Serial communication 115200 baud

/*By Daniel Morberg*/
/*'Requires 'NRFX_SYSTICK_ENABLED 1' in nrfx_config_nrf5340_application.h and nrfx_systic.c included*/
/*Ni kan behöva köra en "make clean" efter ni inkluderat en ny .c fil. */

/*
From the NRF5340DK documentation:

Button 1    P0.23   i.e. 23
Button 2    P0.24
Button 3    P0.08
Button 4    P0.09
LED 1       P0.28   i.e. 28
LED 2       P0.29
LED 3       P0.30
LED 4       P0.31

*/

#define BUTTON1 (23)
#define BUTTON2 (24)
#define BUTTON3 (8)
#define BUTTON4 (9)

#define LED1 (28)
#define LED2 (29)
#define LED3 (30)
#define LED4 (31)

int main(){
    init_uarte();
    nrf_gpio_cfg_output(36); // PIN_TXD_RPI (P1.04)
    nrf_gpio_pin_set(36);    // UART idle state is HIGH
    nrfx_systick_delay_ms(10); // Brief pause so the line settles high
    while (1) {
        print_string_GPIO("Hello World!\r\n");
        nrfx_systick_delay_ms(1000);
    }
    return 0;
}