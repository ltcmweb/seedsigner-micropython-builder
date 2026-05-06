#include "py/obj.h"
#include "py/runtime.h"

uint8_t *pxTaskGetStackStart(TaskHandle_t xTask);
void tinygo_init(void *heap, size_t heap_size, void *glob, void *glob_end, void *stack);
char *mweb(const char *fn, const char *req);
extern char _data_start, _bss_end;

static mp_obj_t mp_mweb(mp_obj_t fn_obj, mp_obj_t req_obj) {
    static bool init;
    if (!init) {
        int heap = 500000;
        void *stack = pxTaskGetStackStart(NULL) + MICROPY_TASK_STACK_SIZE;
        tinygo_init(malloc(heap), heap, &_data_start, &_bss_end, stack);
        init = true;
    }
    char *resp = mweb(mp_obj_str_get_str(fn_obj), mp_obj_str_get_str(req_obj));
    mp_obj_t resp_obj = mp_obj_new_str(resp, strlen(resp));
    free(resp);
    return resp_obj;
}

static MP_DEFINE_CONST_FUN_OBJ_2(mweb_obj, mp_mweb);

static const mp_rom_map_elem_t mweb_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_mweb) },
    { MP_ROM_QSTR(MP_QSTR_mweb), MP_ROM_PTR(&mweb_obj) },
};
static MP_DEFINE_CONST_DICT(mweb_module_globals, mweb_module_globals_table);

const mp_obj_module_t mweb_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mweb_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_mweb_go, mweb_user_cmodule);
