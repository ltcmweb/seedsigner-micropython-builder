extern "C" {
#include "py/obj.h"
#include "py/runtime.h"
}

#include "ur-decoder.hpp"

using namespace ur;

typedef struct {
    mp_obj_base_t base;
    URDecoder *decoder;
} mp_ur_decoder_obj_t;

static mp_obj_t mp_ur_decoder_make_new(const mp_obj_type_t *type, size_t n_args,
                                       size_t n_kw, const mp_obj_t *args) {
    mp_ur_decoder_obj_t *self = mp_obj_malloc_with_finaliser(mp_ur_decoder_obj_t, type);
    self->base.type = type;
    self->decoder = new URDecoder();
    return MP_OBJ_FROM_PTR(self);
}

static mp_obj_t mp_ur_decoder_del(mp_obj_t self_in) {
    mp_ur_decoder_obj_t *self = (mp_ur_decoder_obj_t*)MP_OBJ_TO_PTR(self_in);
    delete self->decoder;
    self->decoder = NULL;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(mp_ur_decoder_del_obj, mp_ur_decoder_del);

static mp_obj_t mp_ur_decoder_receive_part(mp_obj_t self_in, mp_obj_t part_in) {
    mp_ur_decoder_obj_t *self = (mp_ur_decoder_obj_t*)MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_bool(self->decoder->receive_part(mp_obj_str_get_str(part_in)));
}
static MP_DEFINE_CONST_FUN_OBJ_2(mp_ur_decoder_receive_part_obj, mp_ur_decoder_receive_part);

static mp_obj_t mp_ur_decoder_is_complete(mp_obj_t self_in) {
    mp_ur_decoder_obj_t *self = (mp_ur_decoder_obj_t*)MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_bool(self->decoder->is_complete());
}
static MP_DEFINE_CONST_FUN_OBJ_1(mp_ur_decoder_is_complete_obj, mp_ur_decoder_is_complete);

static mp_obj_t mp_ur_decoder_is_success(mp_obj_t self_in) {
    mp_ur_decoder_obj_t *self = (mp_ur_decoder_obj_t*)MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_bool(self->decoder->is_success());
}
static MP_DEFINE_CONST_FUN_OBJ_1(mp_ur_decoder_is_success_obj, mp_ur_decoder_is_success);

static mp_obj_t mp_ur_decoder_progress(mp_obj_t self_in) {
    mp_ur_decoder_obj_t *self = (mp_ur_decoder_obj_t*)MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->decoder->estimated_percent_complete());
}
static MP_DEFINE_CONST_FUN_OBJ_1(mp_ur_decoder_progress_obj, mp_ur_decoder_progress);

static mp_obj_t mp_ur_decoder_result_message(mp_obj_t self_in) {
    mp_ur_decoder_obj_t *self = (mp_ur_decoder_obj_t*)MP_OBJ_TO_PTR(self_in);
    const UR& ur = self->decoder->result_ur();
    mp_obj_t tuple[] = {
        mp_obj_new_str(ur.type().data(), ur.type().size()),
        mp_obj_new_bytes(ur.cbor().data(), ur.cbor().size()),
    };
    return mp_obj_new_tuple(2, tuple);
}
static MP_DEFINE_CONST_FUN_OBJ_1(mp_ur_decoder_result_message_obj, mp_ur_decoder_result_message);

static const mp_rom_map_elem_t mp_ur_decoder_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&mp_ur_decoder_del_obj) },
    { MP_ROM_QSTR(MP_QSTR_receive_part), MP_ROM_PTR(&mp_ur_decoder_receive_part_obj) },
    { MP_ROM_QSTR(MP_QSTR_is_complete), MP_ROM_PTR(&mp_ur_decoder_is_complete_obj) },
    { MP_ROM_QSTR(MP_QSTR_is_success), MP_ROM_PTR(&mp_ur_decoder_is_success_obj) },
    { MP_ROM_QSTR(MP_QSTR_estimated_percent_complete), MP_ROM_PTR(&mp_ur_decoder_progress_obj) },
    { MP_ROM_QSTR(MP_QSTR_result_message), MP_ROM_PTR(&mp_ur_decoder_result_message_obj) },
};

static MP_DEFINE_CONST_DICT(mp_ur_decoder_locals_dict, mp_ur_decoder_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    mp_ur_decoder_type,
    MP_QSTR_URDecoder,
    MP_TYPE_FLAG_NONE,
    make_new, (void*)mp_ur_decoder_make_new,
    locals_dict, &mp_ur_decoder_locals_dict
);

static const mp_rom_map_elem_t mp_module_ur_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_ur) },
    { MP_ROM_QSTR(MP_QSTR_URDecoder), MP_ROM_PTR(&mp_ur_decoder_type) },
};

static MP_DEFINE_CONST_DICT(mp_module_ur_globals, mp_module_ur_globals_table);

extern "C" const mp_obj_module_t mp_module_ur = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_ur_globals,
};

MP_REGISTER_MODULE(MP_QSTR_ur, mp_module_ur);
