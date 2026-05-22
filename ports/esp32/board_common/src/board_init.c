/**
 * Generic board initialisation.
 *
 * Reads board_config.h (selected at build time via BOARD CMake variable)
 * and dispatches to the correct driver init functions.
 */

#include "board.h"
#include "board_config.h"

#include "board_backlight.h"
#include "board_i2c.h"
#include "board_pmic.h"

#include "board_display_axs15231b.h"
#include "board_display_st7701.h"
#include "board_display_st7789.h"
#include "board_display_st7796.h"

#include "board_touch_axs15231b.h"
#include "board_touch_ft6336.h"
#include "board_touch_cst816d.h"
#include "board_touch_gt911.h"

#include "esp_io_expander_tca9554.h"
#include "esp_lcd_axs15231b.h"
#include "esp_lcd_qemu_rgb.h"
#include "esp_lvgl_port.h"

static esp_lcd_panel_handle_t panel_handle;

#if BOARD_DISPLAY_DRIVER == DISPLAY_QEMU
static void qemu_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    if (lv_display_flush_is_last(disp)) {
        void *fb;
        esp_lcd_rgb_qemu_get_frame_buffer(panel_handle, &fb);
        memcpy(fb, px_map, BOARD_LCD_H_RES * BOARD_LCD_V_RES * 2);
        esp_lcd_rgb_qemu_refresh(panel_handle);
    }
    lv_display_flush_ready(disp);
}
#endif

#if BOARD_HAS_IO_EXPANDER
static void io_expander_init(i2c_master_bus_handle_t bus)
{
    esp_io_expander_handle_t expander_handle;

    ESP_ERROR_CHECK(esp_io_expander_new_i2c_tca9554(bus, BOARD_IO_EXPANDER_ADDR, &expander_handle));

    ESP_ERROR_CHECK(esp_io_expander_set_dir(expander_handle, BOARD_IO_EXPANDER_RST_PIN, IO_EXPANDER_OUTPUT));
    ESP_ERROR_CHECK(esp_io_expander_set_level(expander_handle, BOARD_IO_EXPANDER_RST_PIN, 0));
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP_ERROR_CHECK(esp_io_expander_set_level(expander_handle, BOARD_IO_EXPANDER_RST_PIN, 1));
    vTaskDelay(pdMS_TO_TICKS(200));
}
#endif

void boardctrl_startup(void);
esp_err_t esp_psram_init(void);
esp_err_t esp_psram_extram_add_to_heap_allocator(void);

void seedsigner_board_startup(void)
{
    esp_lcd_panel_io_handle_t io_handle;
    esp_lcd_touch_handle_t touch_handle = NULL;
    lv_display_t *disp_out;

    boardctrl_startup();

#if BOARD_DISPLAY_DRIVER == DISPLAY_QEMU
    esp_psram_init();
    esp_psram_extram_add_to_heap_allocator();
    heap_caps_malloc_extmem_enable(CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL);
#endif

    /* Step 1: I2C bus */
    i2c_master_bus_handle_t i2c_bus = board_i2c_init(
        BOARD_PIN_I2C_SDA, BOARD_PIN_I2C_SCL, BOARD_I2C_PORT);

    /* Step 2: IO expander (if present — resets display hardware) */
#if BOARD_HAS_IO_EXPANDER
    io_expander_init(i2c_bus);
#endif

    /* Step 3: Display — must be initialized BEFORE PMIC.
     * The PMIC init changes voltage rails; doing it between the IO expander
     * reset and the SPI panel init can put the display in a bad state.
     * Matches Waveshare demo order: IO expander → Display → PMIC. */
    int display_size = BOARD_LCD_H_RES * BOARD_LCD_V_RES * sizeof(lv_color16_t);
#if BOARD_DISPLAY_DRIVER == DISPLAY_AXS15231B
    board_display_axs15231b_init(&io_handle, &panel_handle, display_size);
#elif BOARD_DISPLAY_DRIVER == DISPLAY_ST7796
    board_display_st7796_init(&io_handle, &panel_handle, display_size);
#elif BOARD_DISPLAY_DRIVER == DISPLAY_ST7789
    board_display_st7789_init(&io_handle, &panel_handle, display_size);
#elif BOARD_DISPLAY_DRIVER == DISPLAY_ST7701
    board_display_st7701_init(&io_handle, &panel_handle);
#elif BOARD_DISPLAY_DRIVER == DISPLAY_QEMU
    esp_lcd_rgb_qemu_config_t panel_config = {
        .width  = BOARD_LCD_V_RES,
        .height = BOARD_LCD_H_RES,
        .bpp    = RGB_QEMU_BPP_16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_rgb_qemu(&panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
#endif

    /* Step 4: PMIC (if present — after display is fully initialized) */
#if BOARD_HAS_PMIC
    board_pmic_init(i2c_bus);
#endif

    /* Step 5: Touch */
#if BOARD_TOUCH_DRIVER == TOUCH_AXS15231B
    touch_handle = board_touch_axs15231b_init(i2c_bus, BOARD_LCD_H_RES, BOARD_LCD_V_RES);
#elif BOARD_TOUCH_DRIVER == TOUCH_FT6336
    touch_handle = board_touch_ft6336_init(i2c_bus, BOARD_LCD_H_RES, BOARD_LCD_V_RES);
#elif BOARD_TOUCH_DRIVER == TOUCH_CST816D
    touch_handle = board_touch_cst816d_init(i2c_bus, BOARD_LCD_H_RES, BOARD_LCD_V_RES);
#elif BOARD_TOUCH_DRIVER == TOUCH_GT911
    touch_handle = board_touch_gt911_init(i2c_bus, BOARD_LCD_H_RES, BOARD_LCD_V_RES);
#endif

    /* Step 6: Backlight — init PWM but keep off (duty=0).
     * Caller turns it on after rendering the first frame to avoid
     * a flash of LVGL's default white background. */
    board_backlight_init(BOARD_PIN_LCD_BL);

    /* Step 7: LVGL port */
    /* Initialize esp_lvgl_port (creates LVGL internals + handler task) */
    lvgl_port_cfg_t port_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    port_cfg.task_priority     = BOARD_LVGL_TASK_PRIORITY;
    port_cfg.task_stack        = BOARD_LVGL_TASK_STACK;
    port_cfg.task_affinity     = BOARD_LVGL_TASK_AFFINITY;
    port_cfg.timer_period_ms   = BOARD_LVGL_TIMER_PERIOD_MS;
    port_cfg.task_max_sleep_ms = BOARD_LVGL_MAX_SLEEP_MS;
    port_cfg.task_stack_caps   = MALLOC_CAP_SPIRAM;
    ESP_ERROR_CHECK(lvgl_port_init(&port_cfg));

    /* ── Register display ── */
#if BOARD_DISPLAY_DRIVER == DISPLAY_ST7701
    /* MIPI-DSI: direct mode with DPI hardware framebuffers.
     * avoid_tearing uses the panel's triple-buffered framebuffers.
     * Landscape rotation is not yet supported — see docs/lvgl-display-rotation.md. */
    lvgl_port_display_cfg_t disp_cfg = {
        .panel_handle = panel_handle,
        .hres         = BOARD_LCD_H_RES,
        .vres         = BOARD_LCD_V_RES,
        .buffer_size  = BOARD_LCD_H_RES * 50,
        .flags = {
            .direct_mode = 1,
        },
    };
    const lvgl_port_display_dsi_cfg_t dsi_cfg = {
        .flags = { .avoid_tearing = 1 },
    };
    disp_out = lvgl_port_add_disp_dsi(&disp_cfg, &dsi_cfg);

#elif BOARD_DISPLAY_DRIVER == DISPLAY_QEMU
    lvgl_port_display_cfg_t disp_cfg = {
        .panel_handle = panel_handle,
        .hres         = BOARD_LCD_H_RES,
        .vres         = BOARD_LCD_V_RES,
        .buffer_size  = BOARD_LCD_H_RES * BOARD_LCD_V_RES,
        .flags = {
            .direct_mode = 1,
        },
    };
    lvgl_port_display_rgb_cfg_t rgb_cfg = { 0 };
    disp_out = lvgl_port_add_disp_rgb(&disp_cfg, &rgb_cfg);
    lv_display_set_flush_cb(disp_out, qemu_flush_cb);

#else
    /* Standard SPI boards (ST7796, ST7789) */
    lvgl_port_display_cfg_t disp_cfg = {
        .io_handle    = io_handle,
        .panel_handle = panel_handle,
        .hres         = BOARD_LCD_H_RES,
        .vres         = BOARD_LCD_V_RES,
        .buffer_size  = BOARD_LCD_H_RES * BOARD_LCD_V_RES,
        .flags = {
            .buff_dma     = 1,
            .buff_spiram  = 1,
            .full_refresh = 1,
            .sw_rotate    = 1,
            .swap_bytes   = 1,
        },
    };
    disp_out = lvgl_port_add_disp(&disp_cfg);
#endif

    lvgl_port_lock(0);
    lv_display_set_rotation(disp_out, LV_DISPLAY_ROTATION_90);
    lvgl_port_unlock();

#if defined(BOARD_DISPLAY_MIRROR_X) && BOARD_DISPLAY_MIRROR_X
    esp_lcd_panel_mirror(panel_handle, true, false);
#endif

    /* Touch input (skip if touch init failed) */
    if (touch_handle) {
        const lvgl_port_touch_cfg_t touch_cfg = {
            .disp   = disp_out,
            .handle = touch_handle,
        };
        lvgl_port_add_touch(&touch_cfg);
    }

    /* Render black screen with backlight off, then turn on backlight.
     * This avoids a flash of LVGL's default white background — the
     * first visible frame is a clean black screen. */
    lvgl_port_lock(0);
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), 0);
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Loading, please wait...");
    lv_obj_set_style_text_color(label, lv_color_make(128, 128, 128), 0);
    lv_obj_center(label);
    lvgl_port_unlock();

    /* Give LVGL time to flush the splash frame before backlight on */
    vTaskDelay(pdMS_TO_TICKS(100));
    board_backlight_set(100);
}

void esp_display_invert(bool enabled)
{
    esp_lcd_panel_invert_color(panel_handle, !enabled);
}
