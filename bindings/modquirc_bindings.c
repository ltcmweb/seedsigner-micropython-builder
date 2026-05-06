#include "py/obj.h"
#include "py/runtime.h"
#include "py/binary.h"

#include "k_quirc.h"

typedef struct _mp_k_quirc_obj_t {
    mp_obj_base_t base;
    k_quirc_t *q;
} mp_k_quirc_obj_t;

static mp_obj_t mp_k_quirc_make_new(const mp_obj_type_t *type,
                                    size_t n_args, size_t n_kw,
                                    const mp_obj_t *args) {
    mp_k_quirc_obj_t *self = mp_obj_malloc(mp_k_quirc_obj_t, type);
    self->q = k_quirc_new();
    if (!self->q) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("k_quirc alloc failed"));
    }
    return MP_OBJ_FROM_PTR(self);
}

static mp_obj_t mp_k_quirc_resize(mp_obj_t self_in, mp_obj_t w_in, mp_obj_t h_in) {
    mp_k_quirc_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int w = mp_obj_get_int(w_in);
    int h = mp_obj_get_int(h_in);

    int r = k_quirc_resize(self->q, w, h);
    return mp_obj_new_int(r);
}
static MP_DEFINE_CONST_FUN_OBJ_3(mp_k_quirc_resize_obj, mp_k_quirc_resize);

static mp_obj_t mp_k_quirc_begin(size_t n_args, const mp_obj_t *args) {
    mp_k_quirc_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    int w = 0, h = 0;
    uint8_t *buf = k_quirc_begin(self->q, &w, &h);

    mp_obj_t tup[3];
    tup[0] = mp_obj_new_bytearray_by_ref(w * h, buf);
    tup[1] = mp_obj_new_int(w);
    tup[2] = mp_obj_new_int(h);

    return mp_obj_new_tuple(3, tup);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_k_quirc_begin_obj, 1, 1, mp_k_quirc_begin);

static mp_obj_t mp_k_quirc_end(mp_obj_t self_in, mp_obj_t inverted_in) {
    mp_k_quirc_obj_t *self = MP_OBJ_TO_PTR(self_in);
    bool inv = mp_obj_is_true(inverted_in);

    k_quirc_end(self->q, inv);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(mp_k_quirc_end_obj, mp_k_quirc_end);

static mp_obj_t mp_k_quirc_count(mp_obj_t self_in) {
    mp_k_quirc_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(k_quirc_count(self->q));
}
static MP_DEFINE_CONST_FUN_OBJ_1(mp_k_quirc_count_obj, mp_k_quirc_count);

static mp_obj_t mp_k_quirc_decode(size_t n_args, const mp_obj_t *args) {
    mp_k_quirc_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int idx = mp_obj_get_int(args[1]);

    k_quirc_result_t res;
    k_quirc_error_t err = k_quirc_decode(self->q, idx, &res);

    mp_obj_t out[5];

    mp_obj_t corners[4];
    for (int i = 0; i < 4; i++) {
        mp_obj_t p[2] = {
            mp_obj_new_int(res.corners[i].x),
            mp_obj_new_int(res.corners[i].y),
        };
        corners[i] = mp_obj_new_tuple(2, p);
    }

    mp_obj_t payload = mp_obj_new_bytes(res.data.payload, res.data.payload_len);

    out[0] = mp_obj_new_int(err);
    out[1] = mp_obj_new_int(res.data.data_type);
    out[2] = payload;
    out[3] = mp_obj_new_int(res.data.payload_len);
    out[4] = mp_obj_new_tuple(4, corners);

    return mp_obj_new_tuple(5, out);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_k_quirc_decode_obj, 2, 2, mp_k_quirc_decode);

static mp_obj_t mp_k_quirc_decode_gray(size_t n_args, const mp_obj_t *args) {
    int w = mp_obj_get_int(args[1]);
    int h = mp_obj_get_int(args[2]);
    bool inv = mp_obj_is_true(args[3]);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[0], &bufinfo, MP_BUFFER_READ);

    k_quirc_result_t results[8];

    int n = k_quirc_decode_grayscale(
        (const uint8_t *)bufinfo.buf,
        w, h,
        results,
        8,
        inv
    );

    mp_obj_t list = mp_obj_new_list(0, NULL);

    for (int i = 0; i < n; i++) {
        mp_obj_t payload = mp_obj_new_bytes(results[i].data.payload,
                                            results[i].data.payload_len);
        mp_obj_list_append(list, payload);
    }

    return list;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_k_quirc_decode_gray_obj, 4, 4, mp_k_quirc_decode_gray);

static mp_obj_t mp_k_quirc_deinit(mp_obj_t self_in) {
    mp_k_quirc_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (self->q) {
        k_quirc_destroy(self->q);
        self->q = NULL;
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(mp_k_quirc_deinit_obj, mp_k_quirc_deinit);

static const mp_rom_map_elem_t mp_k_quirc_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_resize), MP_ROM_PTR(&mp_k_quirc_resize_obj) },
    { MP_ROM_QSTR(MP_QSTR_begin), MP_ROM_PTR(&mp_k_quirc_begin_obj) },
    { MP_ROM_QSTR(MP_QSTR_end), MP_ROM_PTR(&mp_k_quirc_end_obj) },
    { MP_ROM_QSTR(MP_QSTR_count), MP_ROM_PTR(&mp_k_quirc_count_obj) },
    { MP_ROM_QSTR(MP_QSTR_decode), MP_ROM_PTR(&mp_k_quirc_decode_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&mp_k_quirc_deinit_obj) },
};

static MP_DEFINE_CONST_DICT(mp_k_quirc_locals_dict, mp_k_quirc_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    mp_type_k_quirc,
    MP_QSTR_k_quirc,
    MP_TYPE_FLAG_NONE,
    make_new, mp_k_quirc_make_new,
    locals_dict, &mp_k_quirc_locals_dict
);

static const mp_rom_map_elem_t mp_module_k_quirc_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_k_quirc) },
    { MP_ROM_QSTR(MP_QSTR_k_quirc), MP_ROM_PTR(&mp_type_k_quirc) },
    { MP_ROM_QSTR(MP_QSTR_decode_grayscale), MP_ROM_PTR(&mp_k_quirc_decode_gray_obj) },

    { MP_ROM_QSTR(MP_QSTR_SUCCESS), MP_ROM_INT(K_QUIRC_SUCCESS) },
    { MP_ROM_QSTR(MP_QSTR_ERROR), MP_ROM_INT(-1) },
};

static MP_DEFINE_CONST_DICT(mp_module_k_quirc_globals, mp_module_k_quirc_globals_table);

const mp_obj_module_t mp_module_k_quirc = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_k_quirc_globals,
};

MP_REGISTER_MODULE(MP_QSTR_k_quirc, mp_module_k_quirc);
