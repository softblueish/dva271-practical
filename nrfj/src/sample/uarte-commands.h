#ifndef UARTE_COMMANDS_
#define UARTE_COMMANDS_

#include <nrfx.h>
#include <nrf5340_application.h>
#include <nrfx_config.h>
#include <nrf.h>
#include <nrfx_uarte.h>
#include <nrf_gpio.h>
#include <nrfx_systick.h>
#include <nrfx_rtc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define PIN_TXD     20
#define PIN_RXD     22
#define PIN_TXD_RPI 36
#define PIN_RXD_RPI 37
#define BAUD_RATE   9600UL

void uarte_handler(nrfx_uarte_event_t const *p_event, void *p_context);
void uarte_rpi_handler(nrfx_uarte_event_t const *p_event, void *p_context);

nrfx_uarte_t get_uarte(void);
nrfx_uarte_t get_uarte_rpi(void);

nrfx_uarte_t init_uarte(void);

void print_string(const char *message);
void print_newline(void);
void clear_screen(void);
void print_int(int input);
void read_int(int *output);
void read_string(char *output, int max_length);

char read_char(void);

void uart_bb_tx(unsigned long pin, uint8_t data, unsigned long baudrate);
void print_string_GPIO(const char *message);

#endif
