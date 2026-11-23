#include "uarte-commands.h"

#define PIN_TXD 20
#define PIN_RXD 22
#define CLEAR_SCREEN "\033c"

nrfx_uarte_t saved_instance;
volatile bool tx_done;				//volatile tx_done flag and rx_ready signal flag to set up interrupts
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

		/*The UARTE peripheral has completely finished using
		the transmission buffer. main() may start another TX.*/

		tx_done = true;
		break;
		

	case NRFX_UARTE_EVT_RX_DONE:
		/*Save the received byte for main().
		Do not immediately restart reception here if main()
		needs to consume rx_buffer first.*/
		rx_ready = true;
		break;

	case NRFX_UARTE_EVT_ERROR:
		uart_error_count++;

		/*Allow main() to observe that no transmission is active.
		 The program can then decide whether to retry or stop.*/
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
	//Default Settings and give the RX/TX pins:
	const nrfx_uarte_config_t config = NRFX_UARTE_DEFAULT_CONFIG(PIN_TXD, PIN_RXD);
	nrfx_systick_init();

	//Initiate UARTE:
	nrfx_err_t errr = nrfx_uarte_init(&instance, &config, uarte_handler);
	if (errr != NRFX_SUCCESS)
	{
		while (1)
		{
		}
	}
	if (errr != 0) {/*error handling if required*/ }
    saved_instance = instance;
    return instance;
}

//-------------------------------------------------------------------------------------------

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


void uart_bb_tx(unsigned long pin, uint8_t data, float baudrate)
{
    float bit_time_us = 1000000.0f / baudrate;

    // Start bit
    nrf_gpio_pin_write(pin, 0);
    nrfx_systick_delay_us(bit_time_us);

    // Data bits
    for(uint8_t i = 0; i < 8; i++)
    {
        nrf_gpio_pin_write(pin, (data >> i) & 0x01);

        nrfx_systick_delay_us(bit_time_us);
    }

    // Stop bit
    nrf_gpio_pin_write(pin, 1);
    nrfx_systick_delay_us(bit_time_us);
}