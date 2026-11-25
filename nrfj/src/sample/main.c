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

#define PIN_TXD 20
#define PIN_RXD 22
#define PIN_TXD_RPI 36
#define PIN_RXD_RPI 37

#define BAUD_RATE 9600

int main(){
	// note from ashley
	// for some reason the baud rate refuses to match unless we turn off HFCLKCTRL, which supposedly lowers the NRF clock speed
	NRF_CLOCK->HFCLKCTRL = 0;
	init_uarte();
	nrf_gpio_cfg_output(36);
	nrf_gpio_pin_set(36);
	nrf_gpio_cfg_input(37, NRF_GPIO_PIN_PULLUP);
	nrfx_systick_delay_ms(10);
	while (1) {
		uint8_t message = 0;
		uart_bb_rx(PIN_RXD_RPI, &message, BAUD_RATE);
		print_string_GPIO("\nByte recieved: ");
		/*for (uint8_t i = 0; i < 8; i++)
		{
			if ((message >> i) & 0x01)
				print_string_GPIO("1");
			else
				print_string_GPIO("0");
		}
		print_string_GPIO("\n");*/
		nrfx_systick_delay_ms(1000);
	}
	return 0;
}