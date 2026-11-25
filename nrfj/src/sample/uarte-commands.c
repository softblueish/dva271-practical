#include "uarte-commands.h"

#define PIN_TXD 20
#define PIN_RXD 22
#define PIN_TXD_RPI 36
#define PIN_RXD_RPI 37
#define CLEAR_SCREEN "\033c"

nrfx_uarte_t saved_instance;
volatile bool tx_done;
volatile bool rx_ready;
volatile uint8_t rx_buffer;
volatile unsigned long uart_error_count;

void uarte_handler(nrfx_uarte_event_t const* p_event, void* p_context)
{
	(void)p_context;
	static unsigned long count = 0;
	switch (p_event->type)
	{
	case NRFX_UARTE_EVT_TX_DONE:
		tx_done = true;
		break;
	case NRFX_UARTE_EVT_RX_DONE:
		rx_ready = true;
		break;
	case NRFX_UARTE_EVT_ERROR:
		uart_error_count++;
		tx_done = true;
		break;
	default:
		break;
	}
}

nrfx_uarte_t get_uarte(){
	return saved_instance;
}

nrfx_uarte_t init_uarte() {
	nrfx_uarte_t instance = NRFX_UARTE_INSTANCE(0);
	const nrfx_uarte_config_t config = NRFX_UARTE_DEFAULT_CONFIG(PIN_TXD, PIN_RXD_RPI);
	SystemCoreClockUpdate();
	nrfx_systick_init();
	nrfx_err_t errr = nrfx_uarte_init(&instance, &config, uarte_handler);
	if (errr != NRFX_SUCCESS)
	{
		while (1)
		{
		}
	}
	if (errr != 0) {}
	saved_instance = instance;
	return instance;
}

void print_string(char* message){
	char input[256] = "";
	strcat(input, message);
	for(int i = 0; input[i] != '\0'; i++){
		nrfx_uarte_tx(&saved_instance, &input[i], sizeof(input[i]), 0);
	}
}

void print_newline(){
	print_string("\r\n");
}

void clear_screen(){
	print_string(CLEAR_SCREEN);
}

void print_int(int input){
	char msg[100] = "";
	sprintf(msg, "%d\0", input);
	for(int i = 0; msg[i] != '\0'; i++){
		nrfx_uarte_tx(&saved_instance, &msg[i], sizeof(msg[i]), 0);
	}
}

void read_int(int* output){
	char buffer[100] = "";
	for(int i = 0; i < 100; i++){
		nrfx_uarte_rx(&saved_instance, &buffer[i], sizeof(buffer[i]));
		if(buffer[i] == '\r'){
			buffer[i] = '\0';
			break;
		}
	}
	*output = atoi(buffer);
}

void read_string(char* output, int max_length){
	for(int i = 0; i < max_length; i++){
		nrfx_uarte_rx(&saved_instance, &output[i], sizeof(output[i]));
		if(output[i] == '\r'){
			output[i] = '\0';
			break;
		}
	}
}


void uart_bb_tx(unsigned long pin, uint8_t data, unsigned long baudrate)
{
    unsigned long bit_time_us = 1000000UL / baudrate;

    __disable_irq();

    // Start bit (LOW)
    nrf_gpio_pin_clear(pin);
    nrfx_systick_delay_us(bit_time_us);

    // Data bits (LSB first)
    for (uint8_t i = 0; i < 8; i++)
    {
        if ((data >> i) & 0x01)
            nrf_gpio_pin_set(pin);
        else
            nrf_gpio_pin_clear(pin);
        nrfx_systick_delay_us(bit_time_us);
    }

    // Stop bit (HIGH)
    nrf_gpio_pin_set(pin);
    nrfx_systick_delay_us(bit_time_us);

    __enable_irq();
}

void print_string_GPIO(char* message){
    for(int i = 0; message[i] != '\0'; i++){
        uart_bb_tx(PIN_TXD_RPI, (uint8_t)message[i], 9600);
    }
}

/*void uart_bb_rx(unsigned long pin, uint8_t* data, unsigned long baudrate)
{
	unsigned long bit_time_us = 1000000UL / baudrate;
	__disable_irq();
	while(nrf_gpio_pin_read(pin));
	uint8_t buffer = *data;
	nrfx_systick_delay_us(bit_time_us);
	for (uint8_t i = 0; i < 8; i++)
	{
		uint8_t input = 0x00;
		if (nrf_gpio_pin_read(pin))
			input = 0x01;
		else
			input = 0x00;
		buffer = (buffer >> i);
		buffer = buffer | input;
		buffer = (buffer << i);
		nrfx_systick_delay_us(bit_time_us);
	}
	*data = buffer;
	nrfx_systick_delay_us(bit_time_us);
	__enable_irq();
}*/

void read_char(char* output){
	print_string_GPIO("Waiting for keyboard input...\r\n");
	__disable_irq();
	nrfx_uarte_rx(&saved_instance, output, 1)
	__enable_irq();
	print_string_GPIO("We got an input!\r\n");
}