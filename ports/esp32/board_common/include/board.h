/**
 * Board abstraction interface.
 *
 * Each board is defined by a board_config.h selected at compile time via the
 * BOARD CMake variable.  The generic board_init() in board_init.c dispatches
 * on the config defines to initialise the correct drivers.
 */

/* ── Driver selection enums ── */
#define DISPLAY_ST7796      1
#define DISPLAY_ST7789      2
#define DISPLAY_AXS15231B   3
#define DISPLAY_ST7701      4
#define DISPLAY_QEMU        5

#define TOUCH_FT6336        1
#define TOUCH_CST816D       2
#define TOUCH_AXS15231B     3
#define TOUCH_GT911         4

#define PMIC_AXP2101        1

#define CAMERA_DVP          1
#define CAMERA_CSI          2
