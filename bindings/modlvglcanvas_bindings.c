#include "py/obj.h"
#include "py/runtime.h"
#include "py/binary.h"
#include "mod_lvgl.h"
#include "esp_lvgl_port.h"

typedef struct {
    mp_obj_base_t base;
    Canvas canvas;
} canvas_obj_t;

extern const mp_obj_type_t canvas_type;

static mp_obj_t canvas_make_new(const mp_obj_type_t *type, size_t n_args,
                                size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 2, 3, false);

    const char *mode = mp_obj_str_get_str(args[0]);

    size_t len;
    mp_obj_t *size_items;
    mp_obj_get_array(args[1], &len, &size_items);

    if (len != 2) {
        mp_raise_ValueError(MP_ERROR_TEXT("size must be (w,h)"));
    }

    int w = mp_obj_get_int(size_items[0]);
    int h = mp_obj_get_int(size_items[1]);

    bool visible = n_args > 2 ? mp_obj_is_true(args[2]) : false;

    canvas_obj_t *self = mp_obj_malloc_with_finaliser(canvas_obj_t, type);

    self->canvas.buf = NULL;
    self->canvas.canvas = NULL;

    lvgl_port_lock(0);
    bool ok = mod_lvgl_canvas_init(&self->canvas, mode, w, h, visible);
    lvgl_port_unlock();

    if (!ok) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("canvas_init failed"));
    }

    return MP_OBJ_FROM_PTR(self);
}

static mp_obj_t canvas_del(mp_obj_t self_in) {
    canvas_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if (self->canvas.buf) {
        lv_free(self->canvas.buf);
        self->canvas.buf = NULL;
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(canvas_del_obj, canvas_del);

static mp_obj_t canvas_size(mp_obj_t self_in) {
    canvas_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_obj_t tuple[2] = {
        mp_obj_new_int(self->canvas.w),
        mp_obj_new_int(self->canvas.h),
    };

    return mp_obj_new_tuple(2, tuple);
}
static MP_DEFINE_CONST_FUN_OBJ_1(canvas_size_obj, canvas_size);

static mp_obj_t canvas_tobytes(mp_obj_t self_in) {
    canvas_obj_t *self = MP_OBJ_TO_PTR(self_in);

    return mp_obj_new_bytes((const byte *)self->canvas.buf,
                            self->canvas.size);
}
static MP_DEFINE_CONST_FUN_OBJ_1(canvas_tobytes_obj, canvas_tobytes);

static mp_obj_t canvas_setbytes(mp_obj_t self_in, mp_obj_t data_in) {
    canvas_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(data_in, &bufinfo, MP_BUFFER_READ);

    memcpy(self->canvas.buf, bufinfo.buf, self->canvas.size);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(canvas_setbytes_obj, canvas_setbytes);

static mp_obj_t canvas_copyto(mp_obj_t self_in, mp_obj_t other_in, mp_obj_t pos_in) {
    canvas_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if (!mp_obj_is_type(other_in, &canvas_type)) {
        mp_raise_TypeError(MP_ERROR_TEXT("expected Canvas"));
    }

    canvas_obj_t *other = MP_OBJ_TO_PTR(other_in);

    size_t len;
    mp_obj_t *coords;
    mp_obj_get_array(pos_in, &len, &coords);

    if (len != 2) {
        mp_raise_ValueError(MP_ERROR_TEXT("pos must be (x,y)"));
    }

    int x = mp_obj_get_int(coords[0]);
    int y = mp_obj_get_int(coords[1]);

    lvgl_port_lock(0);
    mod_lvgl_canvas_copyto(&other->canvas, &self->canvas, x, y);
    lvgl_port_unlock();

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(canvas_copyto_obj, canvas_copyto);

static const mp_rom_map_elem_t canvas_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_size), MP_ROM_PTR(&canvas_size_obj) },
    { MP_ROM_QSTR(MP_QSTR_tobytes), MP_ROM_PTR(&canvas_tobytes_obj) },
    { MP_ROM_QSTR(MP_QSTR_setbytes), MP_ROM_PTR(&canvas_setbytes_obj) },
    { MP_ROM_QSTR(MP_QSTR_copyto), MP_ROM_PTR(&canvas_copyto_obj) },
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&canvas_del_obj) },
};

static MP_DEFINE_CONST_DICT(canvas_locals_dict, canvas_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    canvas_type,
    MP_QSTR_Canvas,
    MP_TYPE_FLAG_NONE,
    make_new, canvas_make_new,
    locals_dict, &canvas_locals_dict
);
