/*
 * FreeRTOS Kernel V10.0.0
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software. If you wish to use our Amazon
 * FreeRTOS name, please do so in a fair use way that does not cause confusion.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/*
Changes from V1.2.3

	+ portCPU_CLOSK_HZ definition changed to 8MHz base 10, previously it
	  base 16.
*/

#ifndef PORTMACRO_H
#define PORTMACRO_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
#define portCHAR	char
#define portFLOAT	float
#define portDOUBLE	double
#define portLONG	long
#define portSHORT	int
#define portSTACK_TYPE	uint8_t
#define portBASE_TYPE	uint8_t

typedef portSTACK_TYPE StackType_t;
typedef signed char BaseType_t;
typedef unsigned char UBaseType_t;

#if( configUSE_16_BIT_TICKS == 1 )
	typedef uint16_t TickType_t;
	#define portMAX_DELAY ( TickType_t ) 0xffffU
#else
	typedef uint32_t TickType_t;
	#define portMAX_DELAY ( TickType_t ) 0xffffffffUL
#endif
/*-----------------------------------------------------------*/

/* Critical section management. */

#define portENTER_CRITICAL()    __asm__ __volatile__ (					\
                                        "in __tmp_reg__, __SREG__"		"\n\t"	\
                                        "cli" 					"\n\t"	\
                                        "push __tmp_reg__"			"\n\t"	\
                                        ::: "memory"					\
                                        )


#define portEXIT_CRITICAL()     __asm__ __volatile__ (					\
                                        "pop __tmp_reg__"			"\n\t"	\
                                        "out __SREG__, __tmp_reg__" 		"\n\t"	\
                                        ::: "memory"					\
                                        )


#define portDISABLE_INTERRUPTS()        __asm__ __volatile__ ( "cli" ::: "memory")
#define portENABLE_INTERRUPTS()         __asm__ __volatile__ ( "sei" ::: "memory")

/*-----------------------------------------------------------*/

/**
	Enable the watchdog timer, configuring it for expire after
	(value) timeout (which is a combination of the WDP0
	through WDP3 bits).

	This function is derived from <avr/wdt.h> but enables only
	the interrupt bit (WDIE), rather than the reset bit (WDE).

	Can't find it documented but the WDT, once enabled,
	rolls over and fires a new interrupt each time.

	See also the symbolic constants WDTO_15MS et al.
*/
#define wdt_interrupt_enable(value)						\
                                        __asm__ __volatile__ (				\
                                        "in __tmp_reg__,__SREG__" "\n\t"        \
                                        "cli" "\n\t"                            \
                                        "wdr" "\n\t"                            \
                                        "sts %0,%1" "\n\t"                      \
                                        "out __SREG__,__tmp_reg__" "\n\t"       \
                                        "sts %0,%2" "\n\t"                      \
                                        : /* no outputs */                      \
                                        : "M" (_SFR_MEM_ADDR(_WD_CONTROL_REG)), \
                                        "r" (_BV(_WD_CHANGE_BIT) | _BV(WDE)),   \
                                        "r" ((uint8_t) ((value & 0x08 ? _WD_PS3_MASK : 0x00) |   \
                                        _BV(WDIF) | _BV(WDIE) | (value & 0x07)) )                \
                                        : "r0"                                  \
                                        )

/*-----------------------------------------------------------*/
/**
	Enable the watchdog timer, configuring it for expire after
	(value) timeout (which is a combination of the WDP0
	through WDP3 bits).

	This function is derived from <avr/wdt.h> but enables both
	the reset bit (WDE), and the interrupt bit (WDIE).

	This will ensure that if the interrupt is not serviced
	before the second timeout, the AVR will reset.

	Servicing the interrupt automatically clears it,
	and ensures the AVR does not reset.

	Can't find it documented but the WDT, once enabled,
	rolls over and fires a new interrupt each time.

	See also the symbolic constants WDTO_15MS et al.
*/
#define wdt_interrupt_reset_enable(value)					\
				__asm__ __volatile__ 				\
					"in __tmp_reg__,__SREG__" "\n\t"        \
					"cli" "\n\t"                            \
					"wdr" "\n\t"                            \
					"sts %0,%1" "\n\t"                      \
					"out __SREG__,__tmp_reg__" "\n\t"       \
					"sts %0,%2" "\n\t"                      \
					: /* no outputs */                      \
					: "M" (_SFR_MEM_ADDR(_WD_CONTROL_REG)), \
					"r" (_BV(_WD_CHANGE_BIT) | _BV(WDE)),   \
					"r" ((uint8_t) ((value & 0x08 ? _WD_PS3_MASK : 0x00) | \
					_BV(WDIF) | _BV(WDIE) | _BV(WDE) | (value & 0x07)) )   \
					: "r0"                                  \
					)

/*-----------------------------------------------------------*/

/* Architecture specifics. */
#define portSTACK_GROWTH                ( -1 )
#define portBYTE_ALIGNMENT              1
#define portNOP()                       __asm__ __volatile__ ( "nop" );

#define sleep_reset()                   do { _SLEEP_CONTROL_REG = 0; } while(0) // reset all sleep_mode() configurations.

/* Timing for the scheduler.
 * Watchdog Timer is 128kHz nominal,
 * but 120 kHz at 5V DC and 25 degrees is actually more accurate,
 * from data sheet.
 */
#define portTICK_PERIOD_MS              ( (TickType_t) _BV( portUSE_WDTO + 4 ) )	// Inaccurately assuming 128 kHz Watchdog Timer.

/*-----------------------------------------------------------*/

/* Kernel utilities. */
extern void vPortYield( void )          __attribute__ ( ( naked ) );
#define portYIELD()                     vPortYield()

/*-----------------------------------------------------------*/

#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
/* Task function macros as described on the FreeRTOS.org WEB site. */
// This changed to add .lowtext tag for the linker for ATmega2560 and ATmega2561. To make sure they are loaded in low memory.
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters ) __attribute__ ((section (".lowtext")))
#else
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#endif

#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )

#ifdef __cplusplus
}
#endif

#endif /* PORTMACRO_H */

