/**
 * Display manager — thin MicroPython wrapper around board_common.
 *
 * All hardware init (I2C, display, touch, PMIC, backlight, LVGL port)
 * is handled by board_init(). This file just provides the init() and
 * run_screen() C functions that the MicroPython bindings call.
 */

#include "board.h"
#include "board_backlight.h"
#include "board_config.h"
#include "esp_lvgl_port.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static lv_display_t *lvgl_disp = NULL;
static lv_indev_t *lvgl_touch_indev = NULL;

/**
 * Called from MICROPY_BOARD_STARTUP (before REPL starts).
 * Initializes the display at C-level boot so the SPI restart
 * workaround (for ST7796 boards) happens invisibly during startup.
 */
void boardctrl_startup(void);  /* original NVS/flash init */

void seedsigner_board_startup(void)
{
    boardctrl_startup();
    board_app_config_t cfg = { .landscape = true };
    board_init(&cfg, &lvgl_disp, &lvgl_touch_indev);

    /* Render black screen with backlight off, then turn on backlight.
     * This avoids a flash of LVGL's default white background — the
     * first visible frame is a clean black screen. */
    if (lvgl_disp && lvgl_port_lock(0)) {
        lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), 0);
        lv_obj_t *label = lv_label_create(lv_screen_active());
        lv_label_set_text(label, "Loading, please wait...");
        lv_obj_set_style_text_color(label, lv_color_make(128, 128, 128), 0);
        lv_obj_center(label);
        lvgl_port_unlock();
    }
    /* Give LVGL time to flush the splash frame before backlight on */
    vTaskDelay(pdMS_TO_TICKS(100));
    board_backlight_set(100);
}
