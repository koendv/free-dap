/*
 * Copyright (c) 2020, Alex Taradov <alex@taradov.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _DAP_CONFIG_H_
#define _DAP_CONFIG_H_

/*- Includes ----------------------------------------------------------------*/
#include "py/mphal.h"
#include "genhdr/pins.h"

/*- Definitions -------------------------------------------------------------*/
#define SWCLK_TCK                      pyb_pin_DAP_SWCLK
#define SWDIO_TMS                      pyb_pin_DAP_SWDIO
#define nRESET                         pyb_pin_DAP_SRST

#define DAP_CONFIG_ENABLE_SWD
//#define DAP_CONFIG_ENABLE_JTAG

#define DAP_CONFIG_DEFAULT_PORT        DAP_PORT_SWD
#ifndef DAP_CONFIG_DEFAULT_CLOCK
#define DAP_CONFIG_DEFAULT_CLOCK       1000000 // Hz
#endif

#define DAP_CONFIG_PACKET_SIZE         64
#define DAP_CONFIG_PACKET_COUNT        1

// Set the value to NULL if you want to disable a string
// DAP_CONFIG_PRODUCT_STR must contain "CMSIS-DAP" to be compatible with the standard
#define DAP_CONFIG_VENDOR_STR          "Alex Taradov"
#define DAP_CONFIG_PRODUCT_STR         "Generic CMSIS-DAP Adapter"
#define DAP_CONFIG_SER_NUM_STR         dap_serial_number
#define DAP_CONFIG_FW_VER_STR          "v1.0"
#define DAP_CONFIG_DEVICE_VENDOR_STR   "Micropython " MICROPY_VERSION_STRING
#define DAP_CONFIG_DEVICE_NAME_STR     MICROPY_HW_BOARD_NAME

//#define DAP_CONFIG_RESET_TARGET_FN     target_specific_reset_function

// A value at which dap_clock_test() produces 1 kHz output on the SWCLK pin
extern int dap_config_delay_constant;
#ifndef DAP_DEFAULT_DELAY_CONSTANT
#define DAP_DEFAULT_DELAY_CONSTANT     36000
#endif
#define DAP_CONFIG_DELAY_CONSTANT      dap_config_delay_constant

// A threshold for switching to fast clock (no added delays)
// This is the frequency in Hz produced by dap_clock_test(1) on the SWCLK pin 
extern int dap_config_fast_clock;
#ifndef DAP_DEFAULT_FAST_CLOCK
#define DAP_DEFAULT_FAST_CLOCK         9000000
#endif
#define DAP_CONFIG_FAST_CLOCK          dap_config_fast_clock

/*- Prototypes --------------------------------------------------------------*/
extern char dap_serial_number[];

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_TCK_write(int value)
{
  mp_hal_pin_write(SWCLK_TCK, value);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_TMS_write(int value)
{
  mp_hal_pin_write(SWDIO_TMS, value);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_TDO_write(int value)
{
  (void)value;
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_nTRST_write(int value)
{
  (void)value;
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_nRESET_write(int value)
{
  mp_hal_pin_write(nRESET, value);
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_SWCLK_TCK_read(void)
{
  return mp_hal_pin_read(SWCLK_TCK);
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_SWDIO_TMS_read(void)
{
  return mp_hal_pin_read(SWDIO_TMS);
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_TDI_read(void)
{
  return 0;
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_TDO_read(void)
{
  return 0;
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_nTRST_read(void)
{
  return 0;
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_nRESET_read(void)
{
  return mp_hal_pin_read(nRESET);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_TCK_set(void)
{
  mp_hal_pin_high(SWCLK_TCK);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_TCK_clr(void)
{
  mp_hal_pin_low(SWCLK_TCK);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_TMS_in(void)
{
  mp_hal_pin_input(SWDIO_TMS);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_TMS_out(void)
{
  mp_hal_pin_output(SWDIO_TMS);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SETUP(void)
{
  mp_hal_pin_input(SWCLK_TCK);
  mp_hal_pin_config_speed(SWCLK_TCK, GPIO_SPEED_FREQ_VERY_HIGH);
  mp_hal_pin_input(SWDIO_TMS);
  mp_hal_pin_config(SWDIO_TMS, MP_HAL_PIN_MODE_INPUT, MP_HAL_PIN_PULL_UP, 0);
  mp_hal_pin_config_speed(SWDIO_TMS, GPIO_SPEED_FREQ_VERY_HIGH);
  mp_hal_pin_input(nRESET);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_DISCONNECT(void)
{
  mp_hal_pin_input(SWCLK_TCK);
  mp_hal_pin_input(SWDIO_TMS);
  mp_hal_pin_input(nRESET);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_CONNECT_SWD(void)
{
  mp_hal_pin_output(SWDIO_TMS);
  mp_hal_pin_high(SWDIO_TMS);

  mp_hal_pin_output(SWCLK_TCK);
  mp_hal_pin_high(SWCLK_TCK);

  mp_hal_pin_output(nRESET);
  mp_hal_pin_high(nRESET);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_CONNECT_JTAG(void)
{
  mp_hal_pin_output(SWDIO_TMS);
  mp_hal_pin_high(SWDIO_TMS);

  mp_hal_pin_output(SWCLK_TCK);
  mp_hal_pin_high(SWCLK_TCK);

  mp_hal_pin_output(nRESET);
  mp_hal_pin_high(nRESET);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_LED(int index, int state)
{
  (void)index;
  (void)state;
}

#endif // _DAP_CONFIG_H_

