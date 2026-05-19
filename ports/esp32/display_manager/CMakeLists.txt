set(srcs "display_manager.c")

list(APPEND component_requires
    "board_common"
    "esp_lvgl_port"
    "lvgl"
)

idf_component_register(
    SRCS ${srcs}
    REQUIRES ${component_requires}
)
