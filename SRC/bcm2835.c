// bcm2835.c
// C and C++ support for Broadcom BCM 2835 as used in Raspberry Pi
// http://elinux.org/RPi_Low-level_peripherals
// http://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
//
// Author: Mike McCauley
// Copyright (C) 2011-2013 Mike McCauley
// $Id: bcm2835.c,v 1.14 2013/12/06 22:24:52 mikem Exp mikem $


#include "bcm2835.h"
#include "macros.h"

// This define enables a little test program (by default a blinking output on pin RPI_GPIO_PIN_11)
// You can do some safe, non-destructive testing on any platform with:
// gcc bcm2835.c -D BCM2835_TEST
// ./a.out
//#define BCM2835_TEST

// Uncommenting this define compiles alternative I2C code for the version 1 RPi
// The P1 header I2C pins are connected to SDA0 and SCL0 on V1.
// By default I2C code is generated for the V2 RPi which has SDA1 and SCL1 connected.
// #define I2C_V1

// Pointers to the hardware register bases
volatile uint32_t *bcm2835_gpio = (volatile uint32_t *)MAP_FAILED;
volatile uint32_t *bcm2835_pwm  = (volatile uint32_t *)MAP_FAILED;
volatile uint32_t *bcm2835_clk  = (volatile uint32_t *)MAP_FAILED;
volatile uint32_t *bcm2835_pads = (volatile uint32_t *)MAP_FAILED;
volatile uint32_t *bcm2835_spi0 = (volatile uint32_t *)MAP_FAILED;
volatile uint32_t *bcm2835_bsc0 = (volatile uint32_t *)MAP_FAILED;
volatile uint32_t *bcm2835_bsc1 = (volatile uint32_t *)MAP_FAILED;
volatile uint32_t *bcm2835_st	= (volatile uint32_t *)MAP_FAILED;
volatile uint32_t *bcm2835_aux	= (volatile uint32_t *)MAP_FAILED;
volatile uint32_t *bcm2835_mail	= (volatile uint32_t *)MAP_FAILED;


// This variable allows us to test on hardware other than RPi.
// It prevents access to the kernel memory, and does not do any peripheral access
// Instead it prints out what it _would_ do if debug were 0
static uint8_t debug = 0;


//
// Low level register access functions
//

void  bcm2835_set_debug(uint8_t d)
{
	debug = d;
}

// safe read from peripheral
uint32_t bcm2835_peri_read(volatile uint32_t* paddr)
{
	// Make sure we dont return the _last_ read which might get lost
	// if subsequent code changes to a different peripheral
	uint32_t ret = *paddr;
	*paddr; // Read without assigneing to an unused variable
	return ret;
}

// read from peripheral without the read barrier
uint32_t bcm2835_peri_read_nb(volatile uint32_t* paddr)
{
	return *paddr;
}

// safe write to peripheral
void bcm2835_peri_write(volatile uint32_t* paddr, uint32_t value)
{
	// Make sure we don't rely on the first write, which may get
	// lost if the previous access was to a different peripheral.
	*paddr = value;
	*paddr = value;
}

// write to peripheral without the write barrier
void bcm2835_peri_write_nb(volatile uint32_t* paddr, uint32_t value)
{
	*paddr = value;
}

// Set/clear only the bits in value covered by the mask
void bcm2835_peri_set_bits(volatile uint32_t* paddr, uint32_t value, uint32_t mask)
{
	uint32_t v = bcm2835_peri_read(paddr);
	v = (v & ~mask) | (value & mask);
	bcm2835_peri_write(paddr, v);
}

//
// Low level convenience functions
//

// Function select
// pin is a BCM2835 GPIO pin number NOT RPi pin number
//      There are 6 control registers, each control the functions of a block
//      of 10 pins.
//      Each control register has 10 sets of 3 bits per GPIO pin:
//
//      000 = GPIO Pin X is an input
//      001 = GPIO Pin X is an output
//      100 = GPIO Pin X takes alternate function 0
//      101 = GPIO Pin X takes alternate function 1
//      110 = GPIO Pin X takes alternate function 2
//      111 = GPIO Pin X takes alternate function 3
//      011 = GPIO Pin X takes alternate function 4
//      010 = GPIO Pin X takes alternate function 5
//
// So the 3 bits for port X are:
//      X / 10 + ((X % 10) * 3)
void bcm2835_gpio_fsel(uint8_t pin, uint8_t mode)
{
	// Function selects are 10 pins per 32 bit word, 3 bits per pin
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPFSEL0/4 + (pin/10);
	uint8_t   shift = (pin % 10) * 3;
	uint32_t  mask = BCM2835_GPIO_FSEL_MASK << shift;
	uint32_t  value = mode << shift;
	bcm2835_peri_set_bits(paddr, value, mask);
}

// Set output pin
void bcm2835_gpio_set(uint8_t pin)
{
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPSET0/4 + pin/32;
	uint8_t shift = pin % 32;
	bcm2835_peri_write(paddr, 1 << shift);
}

// Clear output pin
void bcm2835_gpio_clr(uint8_t pin)
{
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPCLR0/4 + pin/32;
	uint8_t shift = pin % 32;
	bcm2835_peri_write(paddr, 1 << shift);
}

// Set all output pins in the mask
void bcm2835_gpio_set_multi(uint32_t mask)
{
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPSET0/4;
	bcm2835_peri_write(paddr, mask);
}

// Clear all output pins in the mask
void bcm2835_gpio_clr_multi(uint32_t mask)
{
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPCLR0/4;
	bcm2835_peri_write(paddr, mask);
}

// Read input pin
uint8_t bcm2835_gpio_lev(uint8_t pin)
{
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPLEV0/4 + pin/32;
	uint8_t shift = pin % 32;
	uint32_t value = bcm2835_peri_read(paddr);
	return (value & (1 << shift)) ? HIGH : LOW;
}

// See if an event detection bit is set
// Sigh cant support interrupts yet
uint8_t bcm2835_gpio_eds(uint8_t pin)
{
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPEDS0/4 + pin/32;
	uint8_t shift = pin % 32;
	uint32_t value = bcm2835_peri_read(paddr);
	return (value & (1 << shift)) ? HIGH : LOW;
}

// Write a 1 to clear the bit in EDS
void bcm2835_gpio_set_eds(uint8_t pin)
{
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPEDS0/4 + pin/32;
	uint8_t shift = pin % 32;
	uint32_t value = 1 << shift;
	bcm2835_peri_write(paddr, value);
}

// Rising edge detect enable
void bcm2835_gpio_ren(uint8_t pin)
{
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPREN0/4 + pin/32;
	uint8_t shift = pin % 32;
	uint32_t value = 1 << shift;
	bcm2835_peri_set_bits(paddr, value, value);
}
void bcm2835_gpio_clr_ren(uint8_t pin)
{
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPREN0/4 + pin/32;
	uint8_t shift = pin % 32;
	uint32_t value = 1 << shift;
	bcm2835_peri_set_bits(paddr, 0, value);
}

// Falling edge detect enable
void bcm2835_gpio_fen(uint8_t pin)
{
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPFEN0/4 + pin/32;
	uint8_t shift = pin % 32;
	uint32_t value = 1 << shift;
	bcm2835_peri_set_bits(paddr, value, value);
}
void bcm2835_gpio_clr_fen(uint8_t pin)
{
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPFEN0/4 + pin/32;
	uint8_t shift = pin % 32;
	uint32_t value = 1 << shift;
	bcm2835_peri_set_bits(paddr, 0, value);
}

// High detect enable
void bcm2835_gpio_hen(uint8_t pin)
{
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPHEN0/4 + pin/32;
	uint8_t shift = pin % 32;
	uint32_t value = 1 << shift;
	bcm2835_peri_set_bits(paddr, value, value);
}
void bcm2835_gpio_clr_hen(uint8_t pin)
{
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPHEN0/4 + pin/32;
	uint8_t shift = pin % 32;
	uint32_t value = 1 << shift;
	bcm2835_peri_set_bits(paddr, 0, value);
}

// Low detect enable
void bcm2835_gpio_len(uint8_t pin)
{
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPLEN0/4 + pin/32;
	uint8_t shift = pin % 32;
	uint32_t value = 1 << shift;
	bcm2835_peri_set_bits(paddr, value, value);
}
void bcm2835_gpio_clr_len(uint8_t pin)
{
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPLEN0/4 + pin/32;
	uint8_t shift = pin % 32;
	uint32_t value = 1 << shift;
	bcm2835_peri_set_bits(paddr, 0, value);
}

// Async rising edge detect enable
void bcm2835_gpio_aren(uint8_t pin)
{
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPAREN0/4 + pin/32;
	uint8_t shift = pin % 32;
	uint32_t value = 1 << shift;
	bcm2835_peri_set_bits(paddr, value, value);
}
void bcm2835_gpio_clr_aren(uint8_t pin)
{
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPAREN0/4 + pin/32;
	uint8_t shift = pin % 32;
	uint32_t value = 1 << shift;
	bcm2835_peri_set_bits(paddr, 0, value);
}

// Async falling edge detect enable
void bcm2835_gpio_afen(uint8_t pin)
{
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPAFEN0/4 + pin/32;
	uint8_t shift = pin % 32;
	uint32_t value = 1 << shift;
	bcm2835_peri_set_bits(paddr, value, value);
}
void bcm2835_gpio_clr_afen(uint8_t pin)
{
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPAFEN0/4 + pin/32;
	uint8_t shift = pin % 32;
	uint32_t value = 1 << shift;
	bcm2835_peri_set_bits(paddr, 0, value);
}

// Set pullup/down
void bcm2835_gpio_pud(uint8_t pud)
{
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPPUD/4;
	bcm2835_peri_write(paddr, pud);
}

// Pullup/down clock
// Clocks the value of pud into the GPIO pin
void bcm2835_gpio_pudclk(uint8_t pin, uint8_t on)
{
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPPUDCLK0/4 + pin/32;
	uint8_t shift = pin % 32;
	bcm2835_peri_write(paddr, (on ? 1 : 0) << shift);
}

// Read GPIO pad behaviour for groups of GPIOs
uint32_t bcm2835_gpio_pad(uint8_t group)
{
	volatile uint32_t* paddr = bcm2835_pads + BCM2835_PADS_GPIO_0_27/4 + group*2;
	return bcm2835_peri_read(paddr);
}

// Set GPIO pad behaviour for groups of GPIOs
// powerup value for al pads is
// BCM2835_PAD_SLEW_RATE_UNLIMITED | BCM2835_PAD_HYSTERESIS_ENABLED | BCM2835_PAD_DRIVE_8mA
void bcm2835_gpio_set_pad(uint8_t group, uint32_t control)
{
	volatile uint32_t* paddr = bcm2835_pads + BCM2835_PADS_GPIO_0_27/4 + group*2;
	bcm2835_peri_write(paddr, control | BCM2835_PAD_PASSWRD);
}

// Some convenient arduino-like functions
// milliseconds
void bcm2835_delay(unsigned int millis)
{
	bcm2835_delayMicroseconds(1000 * millis);
}

// microseconds
void bcm2835_delayMicroseconds(uint64_t micros)
{
	uint64_t start = bcm2835_st_read();
	bcm2835_st_delay(start, micros);
}

//
// Higher level convenience functions
//

// Set the state of an output
void bcm2835_gpio_write(uint8_t pin, uint8_t on)
{
	if (on)
		bcm2835_gpio_set(pin);
	else
		bcm2835_gpio_clr(pin);
}

// Set the state of a all 32 outputs in the mask to on or off
void bcm2835_gpio_write_multi(uint32_t mask, uint8_t on)
{
	if (on)
		bcm2835_gpio_set_multi(mask);
	else
		bcm2835_gpio_clr_multi(mask);
}

// Set the state of a all 32 outputs in the mask to the values in value
void bcm2835_gpio_write_mask(uint32_t value, uint32_t mask)
{
	bcm2835_gpio_set_multi(value & mask);
	bcm2835_gpio_clr_multi((~value) & mask);
}

// Set the pullup/down resistor for a pin
//
// The GPIO Pull-up/down Clock Registers control the actuation of internal pull-downs on
// the respective GPIO pins. These registers must be used in conjunction with the GPPUD
// register to effect GPIO Pull-up/down changes. The following sequence of events is
// required:
// 1. Write to GPPUD to set the required control signal (i.e. Pull-up or Pull-Down or neither
// to remove the current Pull-up/down)
// 2. Wait 150 cycles ? this provides the required set-up time for the control signal
// 3. Write to GPPUDCLK0/1 to clock the control signal into the GPIO pads you wish to
// modify ? NOTE only the pads which receive a clock will be modified, all others will
// retain their previous state.
// 4. Wait 150 cycles ? this provides the required hold time for the control signal
// 5. Write to GPPUD to remove the control signal
// 6. Write to GPPUDCLK0/1 to remove the clock
//
// RPi has P1-03 and P1-05 with 1k8 pullup resistor
void bcm2835_gpio_set_pud(uint8_t pin, uint8_t pud)
{
	bcm2835_gpio_pud(pud);
	delayMicroseconds(10);
	bcm2835_gpio_pudclk(pin, 1);
	delayMicroseconds(10);
	bcm2835_gpio_pud(BCM2835_GPIO_PUD_OFF);
	bcm2835_gpio_pudclk(pin, 0);
}

void bcm2835_spi_begin(void)
{
	// Set the SPI0 pins to the Alt 0 function to enable SPI0 access on them
	bcm2835_gpio_fsel(RPI_GPIO_P1_26, BCM2835_GPIO_FSEL_ALT0); // CE1
	bcm2835_gpio_fsel(RPI_GPIO_P1_24, BCM2835_GPIO_FSEL_ALT0); // CE0
	bcm2835_gpio_fsel(RPI_GPIO_P1_21, BCM2835_GPIO_FSEL_ALT0); // MISO
	bcm2835_gpio_fsel(RPI_GPIO_P1_19, BCM2835_GPIO_FSEL_ALT0); // MOSI
	bcm2835_gpio_fsel(RPI_GPIO_P1_23, BCM2835_GPIO_FSEL_ALT0); // CLK

	// Set the SPI CS register to the some sensible defaults
	volatile uint32_t* paddr = bcm2835_spi0 + BCM2835_SPI0_CS/4;
	bcm2835_peri_write(paddr, 0); // All 0s

	// Clear TX and RX fifos
	bcm2835_peri_write_nb(paddr, BCM2835_SPI0_CS_CLEAR);
}

void bcm2835_spi_end(void)
{
	// Set all the SPI0 pins back to input
	bcm2835_gpio_fsel(RPI_GPIO_P1_26, BCM2835_GPIO_FSEL_INPT); // CE1
	bcm2835_gpio_fsel(RPI_GPIO_P1_24, BCM2835_GPIO_FSEL_INPT); // CE0
	bcm2835_gpio_fsel(RPI_GPIO_P1_21, BCM2835_GPIO_FSEL_INPT); // MISO
	bcm2835_gpio_fsel(RPI_GPIO_P1_19, BCM2835_GPIO_FSEL_INPT); // MOSI
	bcm2835_gpio_fsel(RPI_GPIO_P1_23, BCM2835_GPIO_FSEL_INPT); // CLK
}

void bcm2835_spi_setBitOrder(uint8_t order)
{
	// BCM2835_SPI_BIT_ORDER_MSBFIRST is the only one suported by SPI0
}

// defaults to 0, which means a divider of 65536.
// The divisor must be a power of 2. Odd numbers
// rounded down. The maximum SPI clock rate is
// of the APB clock
void bcm2835_spi_setClockDivider(uint16_t divider)
{
	volatile uint32_t* paddr = bcm2835_spi0 + BCM2835_SPI0_CLK/4;
	bcm2835_peri_write(paddr, divider);
}

void bcm2835_spi_setDataMode(uint8_t mode)
{
	volatile uint32_t* paddr = bcm2835_spi0 + BCM2835_SPI0_CS/4;
	// Mask in the CPO and CPHA bits of CS
	bcm2835_peri_set_bits(paddr, mode << 2, BCM2835_SPI0_CS_CPOL | BCM2835_SPI0_CS_CPHA);
}

// Writes (and reads) a single byte to SPI
uint8_t bcm2835_spi_transfer(uint8_t value)
{
	volatile uint32_t* paddr = bcm2835_spi0 + BCM2835_SPI0_CS/4;
	volatile uint32_t* fifo = bcm2835_spi0 + BCM2835_SPI0_FIFO/4;

	// This is Polled transfer as per section 10.6.1
	// BUG ALERT: what happens if we get interupted in this section, and someone else
	// accesses a different peripheral? 
	// Clear TX and RX fifos
	bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);

	// Set TA = 1
	bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);

	// Maybe wait for TXD
	while (!(bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_TXD))
		;

	// Write to FIFO, no barrier
	bcm2835_peri_write_nb(fifo, value);

	// Wait for DONE to be set
	while (!(bcm2835_peri_read_nb(paddr) & BCM2835_SPI0_CS_DONE))
		;

	// Read any byte that was sent back by the slave while we sere sending to it
	uint32_t ret = bcm2835_peri_read_nb(fifo);

	// Set TA = 0, and also set the barrier
	bcm2835_peri_set_bits(paddr, 0, BCM2835_SPI0_CS_TA);

	return ret;
}

// Writes (and reads) an number of bytes to SPI
void bcm2835_spi_transfernb(char* tbuf, char* rbuf, uint32_t len)
{
	volatile uint32_t* paddr = bcm2835_spi0 + BCM2835_SPI0_CS/4;
	volatile uint32_t* fifo = bcm2835_spi0 + BCM2835_SPI0_FIFO/4;
	uint32_t TXCnt=0;
	uint32_t RXCnt=0;

	// This is Polled transfer as per section 10.6.1
	// BUG ALERT: what happens if we get interupted in this section, and someone else
	// accesses a different peripheral? 

	// Clear TX and RX fifos
	bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);

	// Set TA = 1
	bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);

	// Use the FIFO's to reduce the interbyte times
	while((TXCnt < len)||(RXCnt < len))
	{
		// TX fifo not full, so add some more bytes
		while(((bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_TXD))&&(TXCnt < len ))
		{
			bcm2835_peri_write_nb(fifo, tbuf[TXCnt]);
			TXCnt++;
		}
		//Rx fifo not empty, so get the next received bytes
		while(((bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_RXD))&&( RXCnt < len ))
		{
			rbuf[RXCnt] = bcm2835_peri_read_nb(fifo);
			RXCnt++;
		}
	}
	// Wait for DONE to be set
	while (!(bcm2835_peri_read_nb(paddr) & BCM2835_SPI0_CS_DONE))
		;

	// Set TA = 0, and also set the barrier
	bcm2835_peri_set_bits(paddr, 0, BCM2835_SPI0_CS_TA);
}

// Writes an number of bytes to SPI
void bcm2835_spi_writenb(char* tbuf, uint32_t len)
{
	volatile uint32_t* paddr = bcm2835_spi0 + BCM2835_SPI0_CS/4;
	volatile uint32_t* fifo = bcm2835_spi0 + BCM2835_SPI0_FIFO/4;

	// This is Polled transfer as per section 10.6.1
	// BUG ALERT: what happens if we get interupted in this section, and someone else
	// accesses a different peripheral?

	// Clear TX and RX fifos
	bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);

	// Set TA = 1
	bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);

	uint32_t i;
	for (i = 0; i < len; i++)
	{
		// Maybe wait for TXD
		while (!(bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_TXD))
			;

		// Write to FIFO, no barrier
		bcm2835_peri_write_nb(fifo, tbuf[i]);

		// Read from FIFO to prevent stalling
		while (bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_RXD)
			(void) bcm2835_peri_read_nb(fifo);
	}

	// Wait for DONE to be set
	while (!(bcm2835_peri_read_nb(paddr) & BCM2835_SPI0_CS_DONE)) {
		while (bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_RXD)
			(void) bcm2835_peri_read_nb(fifo);
	};

	// Set TA = 0, and also set the barrier
	bcm2835_peri_set_bits(paddr, 0, BCM2835_SPI0_CS_TA);
}

// Writes (and reads) an number of bytes to SPI
// Read bytes are copied over onto the transmit buffer
void bcm2835_spi_transfern(char* buf, uint32_t len)
{
	bcm2835_spi_transfernb(buf, buf, len);
}

void bcm2835_spi_chipSelect(uint8_t cs)
{
	volatile uint32_t* paddr = bcm2835_spi0 + BCM2835_SPI0_CS/4;
	// Mask in the CS bits of CS
	bcm2835_peri_set_bits(paddr, cs, BCM2835_SPI0_CS_CS);
}

void bcm2835_spi_setChipSelectPolarity(uint8_t cs, uint8_t active)
{
	volatile uint32_t* paddr = bcm2835_spi0 + BCM2835_SPI0_CS/4;
	uint8_t shift = 21 + cs;
	// Mask in the appropriate CSPOLn bit
	bcm2835_peri_set_bits(paddr, active << shift, 1 << shift);
}

// Read the System Timer Counter (64-bits)
uint64_t bcm2835_st_read(void)
{
	volatile uint32_t* paddr;
	uint64_t st;
	paddr = bcm2835_st + BCM2835_ST_CHI/4;
	st = bcm2835_peri_read(paddr);
	st <<= 32;
	paddr = bcm2835_st + BCM2835_ST_CLO/4;
	st += bcm2835_peri_read(paddr);
	return st;
}

// Delays for the specified number of microseconds with offset
void bcm2835_st_delay(uint64_t offset_micros, uint64_t micros)
{
	uint64_t compare = offset_micros + micros;

	while(bcm2835_st_read() < compare)
		;
}

// PWM

void bcm2835_pwm_set_clock(uint32_t divisor)
{
	// From Gerts code
	divisor &= 0xfff;
	// Stop PWM clock
	bcm2835_peri_write(bcm2835_clk + BCM2835_PWMCLK_CNTL, BCM2835_PWM_PASSWRD | 0x01);
	bcm2835_delay(110); // Prevents clock going slow
	// Wait for the clock to be not busy
	while ((bcm2835_peri_read(bcm2835_clk + BCM2835_PWMCLK_CNTL) & 0x80) != 0)
		bcm2835_delay(1); 
	// set the clock divider and enable PWM clock
	bcm2835_peri_write(bcm2835_clk + BCM2835_PWMCLK_DIV, BCM2835_PWM_PASSWRD | (divisor << 12));
	bcm2835_peri_write(bcm2835_clk + BCM2835_PWMCLK_CNTL, BCM2835_PWM_PASSWRD | 0x11); // Source=osc and enable
}

void bcm2835_pwm_set_mode(uint8_t channel, uint8_t markspace, uint8_t enabled)
{
	uint32_t control = bcm2835_peri_read(bcm2835_pwm + BCM2835_PWM_CONTROL);

	if (channel == 0)
	{
		if (markspace)
			control |= BCM2835_PWM0_MS_MODE;
		else
			control &= ~BCM2835_PWM0_MS_MODE;
		if (enabled)
			control |= BCM2835_PWM0_ENABLE;
		else
			control &= ~BCM2835_PWM0_ENABLE;
	}
	else if (channel == 1)
	{
		if (markspace)
			control |= BCM2835_PWM1_MS_MODE;
		else
			control &= ~BCM2835_PWM1_MS_MODE;
		if (enabled)
			control |= BCM2835_PWM1_ENABLE;
		else
			control &= ~BCM2835_PWM1_ENABLE;
	}

	// If you use the barrier here, wierd things happen, and the commands dont work
	bcm2835_peri_write_nb(bcm2835_pwm + BCM2835_PWM_CONTROL, control);
	// bcm2835_peri_write_nb(bcm2835_pwm + BCM2835_PWM_CONTROL, BCM2835_PWM0_ENABLE | BCM2835_PWM1_ENABLE | BCM2835_PWM0_MS_MODE | BCM2835_PWM1_MS_MODE);

}

void bcm2835_pwm_set_range(uint8_t channel, uint32_t range)
{
	if (channel == 0)
		bcm2835_peri_write_nb(bcm2835_pwm + BCM2835_PWM0_RANGE, range);
	else if (channel == 1)
		bcm2835_peri_write_nb(bcm2835_pwm + BCM2835_PWM1_RANGE, range);
}

void bcm2835_pwm_set_data(uint8_t channel, uint32_t data)
{
	if (channel == 0)
		bcm2835_peri_write_nb(bcm2835_pwm + BCM2835_PWM0_DATA, data);
	else if (channel == 1)
		bcm2835_peri_write_nb(bcm2835_pwm + BCM2835_PWM1_DATA, data);
}

// AUX
void bcm2835_aux_muart_init(void)
{
	volatile uint32_t* paddr;
	paddr = bcm2835_aux + BCM2835_AUX_ENABLES/4;
	bcm2835_peri_write(paddr, 1);
	paddr = bcm2835_aux + BCM2835_AUX_MU_IER_REG/4;
	bcm2835_peri_write(paddr, 0);
	paddr = bcm2835_aux + BCM2835_AUX_MU_IIR_REG/4;
	bcm2835_peri_write(paddr, 0);
	paddr = bcm2835_aux + BCM2835_AUX_MU_CNTL_REG/4;
	bcm2835_peri_write(paddr, 0);
	paddr = bcm2835_aux + BCM2835_AUX_MU_LCR_REG/4;
	bcm2835_peri_write(paddr, 3); // wrong in documentation
	paddr = bcm2835_aux + BCM2835_AUX_MU_MCR_REG/4;
	bcm2835_peri_write(paddr, 0);
	paddr = bcm2835_aux + BCM2835_AUX_MU_BAUD_REG/4;
	bcm2835_peri_write(paddr, 270);

	bcm2835_gpio_fsel(RPI_GPIO_P1_08, BCM2835_GPIO_FSEL_ALT5); 
	bcm2835_gpio_fsel(RPI_GPIO_P1_10, BCM2835_GPIO_FSEL_ALT5); 

	paddr = bcm2835_aux + BCM2835_AUX_MU_CNTL_REG/4;
	bcm2835_peri_write(paddr, 3);
}

void bcm2835_aux_muart_transfer(uint8_t value)
{
	volatile uint32_t* paddr;
	paddr = bcm2835_aux + BCM2835_AUX_MU_LSR_REG/4;
	while ((bcm2835_peri_read(paddr) & 0x20) == 0)
		;

	paddr = bcm2835_aux + BCM2835_AUX_MU_IO_REG/4;
	bcm2835_peri_write_nb(paddr, value);
}

void bcm2835_aux_muart_transfernb(char* tbuf)
{
	while (*tbuf) 
	{
		bcm2835_aux_muart_transfer(*tbuf);
		tbuf++;
	}
	bcm2835_aux_muart_transfer(0x0d);
	bcm2835_aux_muart_transfer(0x0a);
}

void bcm2835_aux_muart_transfer_hex(uint32_t value)
{
	uint8_t shift;
	uint8_t v;

	shift = 32;
	do
	{
		shift -= 4;
		v = (value >> shift) & 0xf;
		if (v > 9)
			v += 0x37;
		else
			v += 0x30;
		bcm2835_aux_muart_transfer(v);
	} while (shift != 0);
	bcm2835_aux_muart_transfer(0x20); // space
}

void bcm2835_aux_muart_transfer_hexnl(uint32_t value)
{
	bcm2835_aux_muart_transfer_hex(value);
	// Print new line
	bcm2835_aux_muart_transfer(0x0D);
	bcm2835_aux_muart_transfer(0x0A);
}

void bcm2835_mail_write(uint8_t channel, uint32_t value)
{
	while(mmio_read(BCM2835_MAIL0_BASE + BCM2835_MAIL0_STATUS) & BCM2835_MAIL0_STATUS_MAIL_FULL);
	mmio_write(BCM2835_MAIL0_BASE + BCM2835_MAIL0_WRITE, (value & 0xfffffff0) | (uint32_t)(channel & 0xf));
}

uint32_t bcm2835_mail_read(uint8_t channel)
{
  while(1)
    {
      while(mmio_read(BCM2835_MAIL0_BASE + BCM2835_MAIL0_STATUS) & BCM2835_MAIL0_STATUS_MAIL_EMPTY);

      uint32_t data = mmio_read(BCM2835_MAIL0_BASE + BCM2835_MAIL0_READ);
      uint8_t read_channel = (uint8_t)(data & 0xf);
      if(read_channel == channel)
	return (data & 0xfffffff0);
    }
}

// Initialise this library.
int bcm2835_init(void)
{
	bcm2835_pads = (uint32_t*)BCM2835_GPIO_PADS;
	bcm2835_clk  = (uint32_t*)BCM2835_CLOCK_BASE;
	bcm2835_gpio = (uint32_t*)BCM2835_GPIO_BASE;
	bcm2835_pwm  = (uint32_t*)BCM2835_GPIO_PWM;
	bcm2835_spi0 = (uint32_t*)BCM2835_SPI0_BASE;
	bcm2835_bsc0 = (uint32_t*)BCM2835_BSC0_BASE;
	bcm2835_bsc1 = (uint32_t*)BCM2835_BSC1_BASE;
	bcm2835_st   = (uint32_t*)BCM2835_ST_BASE;
	bcm2835_aux  = (uint32_t*)BCM2835_AUX_BASE;
	bcm2835_mail = (uint32_t*)BCM2835_MAIL0_BASE;
	return 1; // Success
}

// Close this library and deallocate everything
int bcm2835_close(void)
{
	return 1; // Success
}  

