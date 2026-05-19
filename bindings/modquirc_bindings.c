#include "py/obj.h"
#include "py/runtime.h"
#include "k_quirc.h"

static mp_obj_t mp_k_quirc_decode_rgb565(size_t n_args, const mp_obj_t *args) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[0], &bufinfo, MP_BUFFER_READ);

    int width = mp_obj_get_int(args[1]);
    int height = mp_obj_get_int(args[2]);
    bool inv = mp_obj_is_true(args[3]);

    k_quirc_t *q = k_quirc_new();
    if (!q) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("k_quirc_new failed"));
    }
    if (k_quirc_resize(q, width, height) < 0) {
        k_quirc_destroy(q);
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("k_quirc_resize failed"));
    }

    uint8_t *buf = k_quirc_begin(q, NULL, NULL);
    uint16_t *pixels = (uint16_t*)bufinfo.buf;
    for (int n = width * height; n--;) {
        uint16_t p = *pixels++;
        uint8_t r5 = (p >> 11) & 0x1F;
        uint8_t g6 = (p >> 5) & 0x3F;
        uint8_t b5 = p & 0x1F;
        uint8_t r8 = (r5 << 3) | (r5 >> 2);
        uint8_t g8 = (g6 << 2) | (g6 >> 4);
        uint8_t b8 = (b5 << 3) | (b5 >> 2);
        *buf++ = (77 * r8 + 150 * g8 + 29 * b8) >> 8;
    }
    k_quirc_end(q, inv);

    mp_obj_t list = mp_obj_new_list(0, NULL);
    for (int i = 0; i < k_quirc_count(q); i++) {
        k_quirc_result_t result;
        k_quirc_error_t err = k_quirc_decode(q, i, &result);
        if (err == K_QUIRC_SUCCESS) {
            mp_obj_list_append(list, mp_obj_new_bytes(result.data.payload, result.data.payload_len));
        }
    }

    k_quirc_destroy(q);
    return list;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_k_quirc_decode_rgb565_obj, 4, 4, mp_k_quirc_decode_rgb565);

static const mp_rom_map_elem_t mp_module_k_quirc_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_k_quirc) },
    { MP_ROM_QSTR(MP_QSTR_decode_rgb565), MP_ROM_PTR(&mp_k_quirc_decode_rgb565_obj) },
};

static MP_DEFINE_CONST_DICT(mp_module_k_quirc_globals, mp_module_k_quirc_globals_table);

const mp_obj_module_t mp_module_k_quirc = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_k_quirc_globals,
};

MP_REGISTER_MODULE(MP_QSTR_k_quirc, mp_module_k_quirc);
