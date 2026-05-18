#include "py/obj.h"
#include "py/runtime.h"

uint8_t *pxTaskGetStackStart(TaskHandle_t xTask);
void tinygo_init(void *heap, size_t heap_size, void *glob, void *glob_end, void *stack);
char *mweb(const char *fn, const char *req);
extern char _data_start, _bss_end;

static mp_obj_t mp_mweb_init(mp_obj_t stack_size_obj, mp_obj_t heap_size_obj) {
    int stack_size = mp_obj_get_int(stack_size_obj);
    int heap_size = mp_obj_get_int(heap_size_obj);

    void *stack = pxTaskGetStackStart(NULL) + stack_size;
    tinygo_init(malloc(heap_size), heap_size, &_data_start, &_bss_end, stack);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(init_obj, mp_mweb_init);

static mp_obj_t mp_mweb(mp_obj_t fn_obj, mp_obj_t req_obj) {
    char *resp = mweb(mp_obj_str_get_str(fn_obj), mp_obj_str_get_str(req_obj));
    mp_obj_t resp_obj = mp_obj_new_str(resp, strlen(resp));
    free(resp);
    return resp_obj;
}
static MP_DEFINE_CONST_FUN_OBJ_2(mweb_obj, mp_mweb);

static const mp_rom_map_elem_t mweb_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_mweb_go) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&init_obj) },
    { MP_ROM_QSTR(MP_QSTR_mweb), MP_ROM_PTR(&mweb_obj) },
};
static MP_DEFINE_CONST_DICT(mweb_module_globals, mweb_module_globals_table);

const mp_obj_module_t mweb_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mweb_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_mweb_go, mweb_user_cmodule);
