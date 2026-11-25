#include "uarte-commands.h"

/*
 * From the nRF5340DK documentation:
 *
 * Button 1    P0.23   i.e. 23
 * Button 2    P0.24
 * Button 3    P0.08
 * Button 4    P0.09
 * LED 1       P0.28   i.e. 28
 * LED 2       P0.29
 * LED 3       P0.30
 * LED 4       P0.31
 */

#define BUTTON1 (23)
#define BUTTON2 (24)
#define BUTTON3 (8)
#define BUTTON4 (9)

#define LED1 (28)
#define LED2 (29)
#define LED3 (30)
#define LED4 (31)

static void byte_to_binary_string(uint8_t value, char out[9])
{
	for (int bit = 7; bit >= 0; bit--)
	{
		out[7 - bit] = ((value >> bit) & 0x01) ? '1' : '0';
	}
	out[8] = '\0';
}

int main(void)
{
	/* Set HCLK divider to Div1 (128 MHz) before SystemCoreClockUpdate() in init_uarte() */
	NRF_CLOCK->HFCLKCTRL = 0;

	init_uarte();

	/* Configure bit-banged TX pin (P1.04 / pin 36) as output, idle HIGH */
	nrf_gpio_cfg_output(PIN_TXD_RPI);
	nrf_gpio_pin_set(PIN_TXD_RPI);
	nrfx_systick_delay_ms(10);

	while (1)
	{
		char received = read_char();
		char binary[9];

		byte_to_binary_string((uint8_t)received, binary);
		print_string_GPIO(binary);
		print_string_GPIO("\r\n");
	}

	return 0;
}
