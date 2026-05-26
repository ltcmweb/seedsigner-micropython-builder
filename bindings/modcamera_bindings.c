#include "py/obj.h"
#include "py/runtime.h"
#include "board_camera.h"

static mp_obj_t camera_start(mp_obj_t width_in, mp_obj_t height_in) {
    int width = mp_obj_get_int(width_in);
    int height = mp_obj_get_int(height_in);

    esp_err_t err = board_camera_init(NULL, width, height);
    if (err != ESP_OK) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("camera init failed"));
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(camera_start_obj, camera_start);

static mp_obj_t camera_stop() {
    board_camera_deinit();

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(camera_stop_obj, camera_stop);

static mp_obj_t camera_read(mp_obj_t buf_obj, mp_obj_t y_obj) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_obj, &bufinfo, MP_BUFFER_WRITE);
    int x = mp_obj_get_int(y_obj);

    board_camera_frame_t frame;
    esp_err_t err = board_camera_fb_get(&frame, 0);
    if (err != ESP_OK) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("camera read failed"));
    }

    uint16_t *p = (uint16_t*)bufinfo.buf;
    uint16_t *end = (uint16_t*)(bufinfo.buf + bufinfo.len);
    for (; x < frame.width; x++) {
        uint16_t *q = (uint16_t*)frame.buf + x;
        for (int y = 0; y < frame.height; y++) {
            if (p >= end) goto out;
            *p++ = __builtin_bswap16(*q);
            q += frame.width;
        }
    }

out:
    board_camera_fb_return(&frame);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(camera_read_obj, camera_read);

static const mp_rom_map_elem_t camera_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_camera) },
    { MP_ROM_QSTR(MP_QSTR_start), MP_ROM_PTR(&camera_start_obj) },
    { MP_ROM_QSTR(MP_QSTR_stop), MP_ROM_PTR(&camera_stop_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&camera_read_obj) },
};
static MP_DEFINE_CONST_DICT(camera_module_globals, camera_module_globals_table);

const mp_obj_module_t camera_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&camera_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_camera, camera_user_cmodule);
