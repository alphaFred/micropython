/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Philipp Ebensberger
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef MICROPY_INCLUDED_MIMXRT_SDCARD_H
#define MICROPY_INCLUDED_MIMXRT_SDCARD_H

#include "modmachine.h"
#include "pin.h"
#include "fsl_usdhc.h"

// ---
// SD Card public defines
// ---
#define SDCARD_DEFAULT_BLOCK_SIZE    (512U)
#define SDCARD_CLOCK_400KHZ          (400000U)
#define SDCARD_CLOCK_25MHZ           (25000000U)
#define SDCARD_CLOCK_50MHZ           (50000000U)



typedef struct _mimxrt_sdcard_pin_t {
    const machine_pin_obj_t *pin;
    uint8_t af_idx;
} mimxrt_sdcard_pin_t;


typedef struct _mimxrt_sdcard_obj_pins_t {
    mimxrt_sdcard_pin_t cmd;
    mimxrt_sdcard_pin_t clk;
    mimxrt_sdcard_pin_t cd_b;
    mimxrt_sdcard_pin_t data0;
    mimxrt_sdcard_pin_t data1;
    mimxrt_sdcard_pin_t data2;
    mimxrt_sdcard_pin_t data3;
} mimxrt_sdcard_obj_pins_t;

typedef struct _mimxrt_sdcard_obj_t {
    mp_obj_base_t base;
    USDHC_Type *usdhc_inst;
    bool initialized;
    uint16_t block_len;
    uint32_t base_clk;
    uint32_t bus_clk;
    uint32_t rca;
    uint32_t block_count;
    uint32_t status;
    uint32_t oper_cond;
    const mimxrt_sdcard_obj_pins_t* pins;
} mimxrt_sdcard_obj_t;


// ---
// SD Card object instances
// ---
extern mimxrt_sdcard_obj_t mimxrt_sdcard_objs[2];


// ---
// SD Card functions
// ---
void sdcard_init_pins(mimxrt_sdcard_obj_t *card);
bool sdcard_read(mimxrt_sdcard_obj_t *card, uint8_t *buffer, uint32_t block_num, uint32_t block_count);
bool sdcard_write(mimxrt_sdcard_obj_t *card, uint8_t *buffer, uint32_t block_num, uint32_t block_count);
bool sdcard_reset(mimxrt_sdcard_obj_t *card);
bool sdcard_set_active(mimxrt_sdcard_obj_t *card);
bool sdcard_volt_validation(mimxrt_sdcard_obj_t *card);
bool sdcard_power_on(mimxrt_sdcard_obj_t *self);
bool sdcard_power_off(mimxrt_sdcard_obj_t *self);
bool sdcard_detect(mimxrt_sdcard_obj_t *self);

#endif // MICROPY_INCLUDED_MIMXRT_SDCARD_H