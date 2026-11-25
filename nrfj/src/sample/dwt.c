#include <dwt.h>
#include <nrf.h>

void dwt_init()
{
    /* Starta DWT-cykelräknaren */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t dwt_get_cycle()
{
    /* Returnerar nuvarande cykelräknarvärde */
    return DWT->CYCCNT;
}

void dwt_delay_until_cycle(uint32_t cycle)
{
    /* Vänta tills cykelräknaren når det angivna värdet */
    while ((int32_t)(DWT->CYCCNT - cycle) < 0) { }
}