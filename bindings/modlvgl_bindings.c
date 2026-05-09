#include "py/obj.h"
#include "py/runtime.h"
#include "py/gc.h"
#include "mod_lvgl.h"
#include "esp_lvgl_port.h"

typedef struct {
    mp_obj_base_t base;
    Canvas canvas;
} canvas_obj_t;

extern const mp_obj_type_t canvas_type;

static mp_obj_t mp_lvgl_screen_size() {
    mp_obj_t tuple[2] = {
        MP_OBJ_NEW_SMALL_INT(lv_obj_get_width(lv_screen_active())),
        MP_OBJ_NEW_SMALL_INT(lv_obj_get_height(lv_screen_active())),
    };

    return mp_obj_new_tuple(2, tuple);
}
static MP_DEFINE_CONST_FUN_OBJ_0(mp_lvgl_screen_size_obj, mp_lvgl_screen_size);

static mp_obj_t mp_lvgl_load(mp_obj_t data_in) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(data_in, &bufinfo, MP_BUFFER_READ);

    canvas_obj_t *obj = mp_obj_malloc_with_finaliser(canvas_obj_t, &canvas_type);

    for (int i = 0; i < 2; i++) {
        lvgl_port_lock(0);
        bool ok = mod_lvgl_load(&obj->canvas, (const uint8_t *)bufinfo.buf, bufinfo.len);
        lvgl_port_unlock();
        if (ok) break;
        if (i) mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("lvgl_load failed"));
        gc_collect();
    }

    return MP_OBJ_FROM_PTR(obj);
}
static MP_DEFINE_CONST_FUN_OBJ_1(mp_lvgl_load_obj, mp_lvgl_load);

static mp_obj_t mp_lvgl_rectangle(size_t n_args, const mp_obj_t *args) {
    if (!mp_obj_is_type(args[0], &canvas_type)) {
        mp_raise_TypeError(MP_ERROR_TEXT("expected Canvas"));
    }

    canvas_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    size_t len;
    mp_obj_t *coords;
    mp_obj_get_array(args[1], &len, &coords);

    if (len != 4) {
        mp_raise_ValueError(MP_ERROR_TEXT("coords must be 4-tuple"));
    }

    int x1 = mp_obj_get_int(coords[0]);
    int y1 = mp_obj_get_int(coords[1]);
    int x2 = mp_obj_get_int(coords[2]);
    int y2 = mp_obj_get_int(coords[3]);

    int fill    = mp_obj_get_int(args[2]);
    int outline = mp_obj_get_int(args[3]);
    int width   = mp_obj_get_int(args[4]);
    int radius  = mp_obj_get_int(args[5]);

    lvgl_port_lock(0);
    mod_lvgl_rect(&self->canvas, x1, y1, x2, y2, fill, outline, width, radius);
    lvgl_port_unlock();

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_lvgl_rectangle_obj, 6, 6, mp_lvgl_rectangle);

static mp_obj_t mp_lvgl_line(size_t n_args, const mp_obj_t *args) {
    if (!mp_obj_is_type(args[0], &canvas_type)) {
        mp_raise_TypeError(MP_ERROR_TEXT("expected Canvas"));
    }

    canvas_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    size_t len;
    mp_obj_t *coords;
    mp_obj_get_array(args[1], &len, &coords);

    if (len != 4) {
        mp_raise_ValueError(MP_ERROR_TEXT("coords must be 4-tuple"));
    }

    int x1 = mp_obj_get_int(coords[0]);
    int y1 = mp_obj_get_int(coords[1]);
    int x2 = mp_obj_get_int(coords[2]);
    int y2 = mp_obj_get_int(coords[3]);

    int fill = mp_obj_get_int(args[2]);

    lvgl_port_lock(0);
    mod_lvgl_line(&self->canvas, x1, y1, x2, y2, fill);
    lvgl_port_unlock();

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_lvgl_line_obj, 3, 3, mp_lvgl_line);

static mp_obj_t mp_lvgl_arc(size_t n_args, const mp_obj_t *args) {
    if (!mp_obj_is_type(args[0], &canvas_type)) {
        mp_raise_TypeError(MP_ERROR_TEXT("expected Canvas"));
    }

    canvas_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    size_t len;
    mp_obj_t *coords;
    mp_obj_get_array(args[1], &len, &coords);

    if (len != 4) {
        mp_raise_ValueError(MP_ERROR_TEXT("coords must be 4-tuple"));
    }

    int x1 = mp_obj_get_int(coords[0]);
    int y1 = mp_obj_get_int(coords[1]);
    int x2 = mp_obj_get_int(coords[2]);
    int y2 = mp_obj_get_int(coords[3]);

    int start = mp_obj_get_int(args[2]);
    int end = mp_obj_get_int(args[3]);
    int fill = mp_obj_get_int(args[4]);
    int width = mp_obj_get_int(args[5]);

    lvgl_port_lock(0);
    mod_lvgl_arc(&self->canvas, x1, y1, x2, y2, start, end, fill, width);
    lvgl_port_unlock();

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_lvgl_arc_obj, 6, 6, mp_lvgl_arc);

static mp_obj_t mp_lvgl_text(size_t n_args, const mp_obj_t *args) {
    if (!mp_obj_is_type(args[0], &canvas_type)) {
        mp_raise_TypeError(MP_ERROR_TEXT("expected Canvas"));
    }

    canvas_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    size_t len;
    mp_obj_t *coords;
    mp_obj_get_array(args[1], &len, &coords);

    if (len != 2) {
        mp_raise_ValueError(MP_ERROR_TEXT("coords must be 2-tuple"));
    }

    int x = mp_obj_get_int(coords[0]);
    int y = mp_obj_get_int(coords[1]);

    const char *text = mp_obj_str_get_str(args[2]);
    const char *font = mp_obj_str_get_str(args[3]);
    int fill         = mp_obj_get_int(args[4]);
    const char *anchor = mp_obj_str_get_str(args[5]);

    lv_area_t box;
    lvgl_port_lock(0);
    mod_lvgl_text(&self->canvas, x, y, fill, text, font, anchor, &box);
    lvgl_port_unlock();

    mp_obj_t tuple[4] = {
        MP_OBJ_NEW_SMALL_INT(box.x1),
        MP_OBJ_NEW_SMALL_INT(box.y1),
        MP_OBJ_NEW_SMALL_INT(box.x2),
        MP_OBJ_NEW_SMALL_INT(box.y2),
    };

    return mp_obj_new_tuple(4, tuple);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_lvgl_text_obj, 6, 6, mp_lvgl_text);

static mp_obj_t mp_lvgl_putalpha(size_t n_args, const mp_obj_t *args) {
    if (!mp_obj_is_type(args[0], &canvas_type)) {
        mp_raise_TypeError(MP_ERROR_TEXT("expected Canvas"));
    }

    canvas_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int alpha = mp_obj_get_int(args[1]);

    mod_lvgl_putalpha(&self->canvas, alpha);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_lvgl_putalpha_obj, 2, 2, mp_lvgl_putalpha);

static mp_obj_t input_cb;
static lv_event_code_t last_evt_code;
static lv_point_t last_evt_pt;
static mp_sched_node_t push_input_event_node;

static void push_input_event(mp_sched_node_t *node) {
    mp_obj_t args[] = {
        MP_OBJ_NEW_SMALL_INT(last_evt_code),
        MP_OBJ_NEW_SMALL_INT(last_evt_pt.x),
        MP_OBJ_NEW_SMALL_INT(last_evt_pt.y),
    };

    mp_call_function_n_kw(input_cb, 3, 0, args);
}

static void screen_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    switch (code) {
    case LV_EVENT_PRESSED:
    case LV_EVENT_PRESSING:
    case LV_EVENT_RELEASED:
        break;
    default:
        return;
    }

    lv_point_t pt;
    lv_indev_get_point(lv_indev_active(), &pt);

    if (code == last_evt_code &&
        pt.x == last_evt_pt.x &&
        pt.y == last_evt_pt.y)
        return;

    last_evt_code = code;
    last_evt_pt.x = pt.x;
    last_evt_pt.y = pt.y;

    mp_sched_schedule_node(&push_input_event_node, push_input_event);
}

static mp_obj_t mp_lvgl_register_input_cb(mp_obj_t data_in) {
    input_cb = data_in;

    lvgl_port_lock(0);
    lv_obj_add_event_cb(lv_screen_active(), screen_event_cb, LV_EVENT_ALL, NULL);
    lvgl_port_unlock();

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(mp_lvgl_register_input_cb_obj, mp_lvgl_register_input_cb);

static const mp_rom_map_elem_t lvgl_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_lvgl) },
    { MP_ROM_QSTR(MP_QSTR_screen_size), MP_ROM_PTR(&mp_lvgl_screen_size_obj) },
    { MP_ROM_QSTR(MP_QSTR_load), MP_ROM_PTR(&mp_lvgl_load_obj) },
    { MP_ROM_QSTR(MP_QSTR_rectangle), MP_ROM_PTR(&mp_lvgl_rectangle_obj) },
    { MP_ROM_QSTR(MP_QSTR_line), MP_ROM_PTR(&mp_lvgl_line_obj) },
    { MP_ROM_QSTR(MP_QSTR_arc), MP_ROM_PTR(&mp_lvgl_arc_obj) },
    { MP_ROM_QSTR(MP_QSTR_text), MP_ROM_PTR(&mp_lvgl_text_obj) },
    { MP_ROM_QSTR(MP_QSTR_putalpha), MP_ROM_PTR(&mp_lvgl_putalpha_obj) },
    { MP_ROM_QSTR(MP_QSTR_register_input_cb), MP_ROM_PTR(&mp_lvgl_register_input_cb_obj) },
    { MP_ROM_QSTR(MP_QSTR_Canvas), MP_ROM_PTR(&canvas_type) },
};

static MP_DEFINE_CONST_DICT(lvgl_module_globals, lvgl_module_globals_table);

const mp_obj_module_t lvgl_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&lvgl_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_lvgl, lvgl_user_cmodule);
