#include "uarte-commands.h"
#include "dwt.h"

#define CLEAR_SCREEN "\033c"

static nrfx_uarte_t uarte_pc = NRFX_UARTE_INSTANCE(0);
static nrfx_uarte_t uarte_rpi = NRFX_UARTE_INSTANCE(1);

static volatile bool pc_tx_done = false;
static volatile bool pc_rx_ready = false;
static volatile bool pc_rx_error = false;
static uint8_t pc_rx_dma_byte = 0;
static volatile uint8_t pc_rx_byte = 0;

static volatile bool rpi_rx_ready = false;
static volatile bool rpi_rx_error = false;
static uint8_t rpi_rx_dma_byte = 0;
static volatile uint8_t rpi_rx_byte = 0;

volatile unsigned long uart_error_count = 0;

void uarte_handler(nrfx_uarte_event_t const *p_event, void *p_context)
{
	(void)p_context;

	switch (p_event->type)
	{
	case NRFX_UARTE_EVT_TX_DONE:
		pc_tx_done = true;
		break;
	case NRFX_UARTE_EVT_RX_DONE:
		if (p_event->data.rx.bytes > 0)
		{
			pc_rx_byte = p_event->data.rx.p_data[0];
			pc_rx_ready = true;
		}
		break;
	case NRFX_UARTE_EVT_ERROR:
		uart_error_count++;
		pc_tx_done = true;
		pc_rx_error = true;
		break;
	default:
		break;
	}
}

void uarte_rpi_handler(nrfx_uarte_event_t const *p_event, void *p_context)
{
	(void)p_context;

	switch (p_event->type)
	{
	case NRFX_UARTE_EVT_RX_DONE:
		if (p_event->data.rx.bytes > 0)
		{
			rpi_rx_byte = p_event->data.rx.p_data[0];
			rpi_rx_ready = true;
		}
		break;
	case NRFX_UARTE_EVT_ERROR:
		uart_error_count++;
		rpi_rx_error = true;
		break;
	default:
		break;
	}
}

nrfx_uarte_t get_uarte(void)
{
	return uarte_pc;
}

nrfx_uarte_t get_uarte_rpi(void)
{
	return uarte_rpi;
}

nrfx_uarte_t init_uarte(void)
{
	SystemCoreClockUpdate();
	nrfx_systick_init();

	/* UARTE0: PC USB serial (pins 20 TX, 22 RX) */
	nrfx_uarte_config_t pc_config = NRFX_UARTE_DEFAULT_CONFIG(PIN_TXD, PIN_RXD);
	nrfx_err_t err = nrfx_uarte_init(&uarte_pc, &pc_config, uarte_handler);
	if (err != NRFX_SUCCESS)
	{
		while (1)
		{
		}
	}

	/* UARTE1: Raspberry Pi RX on pin 37 (P1.05).
	 * Hardware TX is disconnected so it cannot conflict with bit-banged TX on pin 36 (P1.04).
	 */
	nrfx_uarte_config_t rpi_config = NRFX_UARTE_DEFAULT_CONFIG(NRF_UARTE_PSEL_DISCONNECTED, PIN_RXD_RPI);
	rpi_config.baudrate = NRF_UARTE_BAUDRATE_9600;
	err = nrfx_uarte_init(&uarte_rpi, &rpi_config, uarte_rpi_handler);
	if (err != NRFX_SUCCESS)
	{
		while (1)
		{
		}
	}

	return uarte_rpi;
}

void print_string(const char *message)
{
	uint8_t tx_buf[256];
	size_t len = strlen(message);
	if (len == 0)
	{
		return;
	}
	if (len > sizeof(tx_buf))
	{
		len = sizeof(tx_buf);
	}
	memcpy(tx_buf, message, len);

	pc_tx_done = false;
	if (nrfx_uarte_tx(&uarte_pc, tx_buf, len, 0) == NRFX_SUCCESS)
	{
		while (!pc_tx_done)
		{
		}
	}
}

void print_newline(void)
{
	print_string("\r\n");
}

void clear_screen(void)
{
	print_string(CLEAR_SCREEN);
}

void print_int(int input)
{
	char msg[32];
	snprintf(msg, sizeof(msg), "%d", input);
	print_string(msg);
}

static char read_char_pc(void)
{
	pc_rx_ready = false;
	pc_rx_error = false;
	nrfx_uarte_rx(&uarte_pc, &pc_rx_dma_byte, 1);

	while (!pc_rx_ready)
	{
		if (pc_rx_error)
		{
			pc_rx_error = false;
			nrfx_uarte_rx(&uarte_pc, &pc_rx_dma_byte, 1);
		}
	}
	return (char)pc_rx_byte;
}

void read_int(int *output)
{
	char buffer[100];
	int i = 0;
	for (; i < (int)sizeof(buffer) - 1; i++)
	{
		buffer[i] = read_char_pc();
		if (buffer[i] == '\r' || buffer[i] == '\n')
		{
			break;
		}
	}
	buffer[i] = '\0';
	*output = atoi(buffer);
}

void read_string(char *output, int max_length)
{
	if (max_length <= 0)
	{
		return;
	}
	int i = 0;
	for (; i < max_length - 1; i++)
	{
		output[i] = read_char_pc();
		if (output[i] == '\r' || output[i] == '\n')
		{
			break;
		}
	}
	output[i] = '\0';
}

char read_char(void)
{
	rpi_rx_ready = false;
	rpi_rx_error = false;
	nrfx_uarte_rx(&uarte_rpi, &rpi_rx_dma_byte, 1);

	while (!rpi_rx_ready)
	{
		if (rpi_rx_error)
		{
			rpi_rx_error = false;
			nrfx_uarte_rx(&uarte_rpi, &rpi_rx_dma_byte, 1);
		}
	}
	return (char)rpi_rx_byte;
}

void uart_bb_tx(unsigned long pin, uint8_t data, unsigned long baudrate)
{
	unsigned long bit_time_us = 1000000UL / baudrate;

	__disable_irq();

	/* Start bit (LOW) */
	nrf_gpio_pin_clear(pin);
	nrfx_systick_delay_us(bit_time_us);

	/* Data bits (LSB first) */
	for (uint8_t i = 0; i < 8; i++)
	{
		if ((data >> i) & 0x01)
		{
			nrf_gpio_pin_set(pin);
		}
		else
		{
			nrf_gpio_pin_clear(pin);
		}
		nrfx_systick_delay_us(bit_time_us);
	}

	/* Stop bit (HIGH) */
	nrf_gpio_pin_set(pin);
	nrfx_systick_delay_us(bit_time_us);

	__enable_irq();
}*/

void print_string_GPIO(const char *message)
{
	for (size_t i = 0; message[i] != '\0'; i++)
	{
		uart_bb_tx(PIN_TXD_RPI, (uint8_t)message[i], BAUD_RATE);
	}
}
