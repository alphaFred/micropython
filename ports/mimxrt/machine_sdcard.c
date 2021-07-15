
/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Philipp Ebensberger
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

#include "py/runtime.h"
#include "py/mperrno.h"
#include "extmod/vfs.h"
#include "fsl_cache.h"

#include "sdcard.h"


#define SDCARD_INIT_ARG_ID          (0)


STATIC const mp_arg_t allowed_args[] = {
    [SDCARD_INIT_ARG_ID]     { MP_QSTR_id, MP_ARG_INT, {.u_int = 1} },
};


STATIC bool machine_sdcard_init_helper(mimxrt_sdcard_obj_t *self, const mp_arg_val_t *args) {
    sdcard_init_pins(self);

    // Initialize USDHC
    const usdhc_config_t config = {
        .endianMode = kUSDHC_EndianModeLittle,
        .dataTimeout = 0xFU,
        .readWatermarkLevel = 128U,
        .writeWatermarkLevel = 128U,
    };
    USDHC_Init(self->usdhc_inst, &config);

    self->initialized = false;
    return sdcard_detect(self);
}

STATIC mp_obj_t sdcard_obj_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);

    // Parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, all_args, &kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // Extract arguments
    mp_int_t sdcard_id = args[SDCARD_INIT_ARG_ID].u_int;

    if (!(1 <= sdcard_id && sdcard_id <= MP_ARRAY_SIZE(mimxrt_sdcard_objs))) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "SDCard(%d) doesn't exist", sdcard_id));
    }

    mimxrt_sdcard_obj_t *self = &mimxrt_sdcard_objs[(sdcard_id - 1)];

    // Initialize SDCard Host
    if (machine_sdcard_init_helper(self, args)) {
        return MP_OBJ_FROM_PTR(self);
    } else {
        return mp_const_none;
    }
}

// init()
STATIC mp_obj_t machine_sdcard_init(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // Parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args) - 1];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // Todo: make sure that there is a valid reason that a call to init would not change anything for an already initialized hsot
    machine_sdcard_init_helper(pos_args[0], args);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_sdcard_init_obj, 1, machine_sdcard_init);

// deinit()
STATIC mp_obj_t machine_sdcard_deinit(mp_obj_t self_in) {
    // TODO: Implement deinit function
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_sdcard_deinit_obj, machine_sdcard_deinit);

// readblocks(block_num, buf, [offset])
STATIC mp_obj_t machine_sdcard_readblocks(mp_obj_t self_in, mp_obj_t _block_num, mp_obj_t _buf) {
    mp_buffer_info_t bufinfo;
    mimxrt_sdcard_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_get_buffer_raise(_buf, &bufinfo, MP_BUFFER_WRITE);
    // ---

    if (self->initialized &&
        sdcard_read(self, bufinfo.buf, mp_obj_get_int(_block_num), bufinfo.len / SDCARD_DEFAULT_BLOCK_SIZE)) {
        return MP_OBJ_NEW_SMALL_INT(0);
    } else {
        // Todo: fix upper layer instead of throwing exception here
        mp_raise_OSError(EIO);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(machine_sdcard_readblocks_obj, machine_sdcard_readblocks);

// writeblocks(block_num, buf, [offset])
STATIC mp_obj_t machine_sdcard_writeblocks(mp_obj_t self_in, mp_obj_t _block_num, mp_obj_t _buf) {
    mp_buffer_info_t bufinfo;
    mimxrt_sdcard_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_get_buffer_raise(_buf, &bufinfo, MP_BUFFER_WRITE);
    // ---

    if (self->initialized &&
        sdcard_write(self, bufinfo.buf, mp_obj_get_int(_block_num), bufinfo.len / SDCARD_DEFAULT_BLOCK_SIZE)) {
        return MP_OBJ_NEW_SMALL_INT(0);
    } else {
        // Todo: fix upper layer instead of throwing exception here
        mp_raise_OSError(EIO);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(machine_sdcard_writeblocks_obj, machine_sdcard_writeblocks);

// ioctl(op, arg)
STATIC mp_obj_t machine_sdcard_ioctl(mp_obj_t self_in, mp_obj_t cmd_in, mp_obj_t arg_in) {
    mimxrt_sdcard_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t cmd = mp_obj_get_int(cmd_in);

    switch (cmd) {
        case MP_BLOCKDEV_IOCTL_INIT: {
            if (sdcard_detect(self) && sdcard_power_on(self)) {
                return MP_OBJ_NEW_SMALL_INT(0);
            } else {
                self->initialized = false;
                return MP_OBJ_NEW_SMALL_INT(-1);  // Initialization failed
            }
        }
        case MP_BLOCKDEV_IOCTL_DEINIT: {
            if (sdcard_power_off(self)) {
                return MP_OBJ_NEW_SMALL_INT(0);
            } else {
                return MP_OBJ_NEW_SMALL_INT(-1);  // Deinitialization failed
            }
        }
        case MP_BLOCKDEV_IOCTL_SYNC: {
            return MP_OBJ_NEW_SMALL_INT(0);
        }
        case MP_BLOCKDEV_IOCTL_BLOCK_COUNT: {
            if (self->initialized) {
                return MP_OBJ_NEW_SMALL_INT(self->block_count);
            } else {
                return MP_OBJ_NEW_SMALL_INT(-1);  // Card not initialized
            }
        }
        case MP_BLOCKDEV_IOCTL_BLOCK_SIZE: {
            if (self->initialized) {
                return MP_OBJ_NEW_SMALL_INT(self->block_len);
            } else {
                return MP_OBJ_NEW_SMALL_INT(-1);  // Card not initialized
            }
        }
        default: // unknown command
        {
            return MP_OBJ_NEW_SMALL_INT(-1); // error
        }
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(machine_sdcard_ioctl_obj, machine_sdcard_ioctl);

STATIC const mp_rom_map_elem_t sdcard_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init),        MP_ROM_PTR(&machine_sdcard_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit),      MP_ROM_PTR(&machine_sdcard_deinit_obj) },
    // block device protocol
    { MP_ROM_QSTR(MP_QSTR_readblocks),  MP_ROM_PTR(&machine_sdcard_readblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_writeblocks), MP_ROM_PTR(&machine_sdcard_writeblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_ioctl),       MP_ROM_PTR(&machine_sdcard_ioctl_obj) },
};
STATIC MP_DEFINE_CONST_DICT(sdcard_locals_dict, sdcard_locals_dict_table);

const mp_obj_type_t machine_sdcard_type = {
    { &mp_type_type },
    .name = MP_QSTR_SDCard,
    .make_new = sdcard_obj_make_new,
    .locals_dict = (mp_obj_dict_t *)&sdcard_locals_dict,
};

void machine_sdcard_init0(void) {
    return;
}
