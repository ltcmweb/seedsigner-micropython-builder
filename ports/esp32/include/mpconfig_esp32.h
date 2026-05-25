#define MICROPY_GC_INITIAL_HEAP_SIZE        (3584 * 1024)
#define MP_PLAT_ALLOC_HEAP                  malloc

#include <mpconfigport.h>

#undef MICROPY_FLOAT_IMPL
#define MICROPY_FLOAT_IMPL                  (MICROPY_FLOAT_IMPL_DOUBLE)

#undef MICROPY_GC_SPLIT_HEAP_AUTO
#define MICROPY_GC_SPLIT_HEAP_AUTO          (0)

#undef MICROPY_PY_MACHINE_ADC
#undef MICROPY_PY_MACHINE_ADC_BLOCK
