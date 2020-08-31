/*
 * Copyright (C) 2005-2009 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <syslib.h>
#include <spilib.h>
#include <itc2.h>

/*
 * \note:
 * TODO: Example 81150/SPI_EX1 is NOT tested on HW yet
 */

volatile uint8 tx_data;

static void hw_init (void);

/*
 * MAIN
 */
int main (void)
{
	hw_init();

	for ( ; ; ) {
        /* Main loop */    
    }

	return 0;
}

/*
 * ISR: SPI RX
 */
void SPI1_RX_Interrupt (void)
{
	uint16 rx_data;

	rx_data = SPI1_DR;
}


/*
 * ISR: SPI TX
 */
void SPI1_TX_Interrupt (void)
{
	SPI1_DR = tx_data++;
}

/*
 *
 */
static void hw_init (void)
{

    /*
     * \note:
     * TODO: SPI IO pins (MISO/MOSI/SCKL/nCS) should be initialized and connected here
     */

	tx_data = 1;

	SPI1_SLAVE_INIT(SPI_FORMAT_00);

    EXT3_INT_ENABLE(3); /* Enable EXT3 line on 1st level interrupt controller */
    SPI1_RI_ENABLE();
    SPI1_TI_ENABLE();

	SPI1_DR = tx_data++;
}

/* EOF */
