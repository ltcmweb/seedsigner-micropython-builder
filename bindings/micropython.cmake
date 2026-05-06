add_library(usermod_dm INTERFACE)

target_sources(usermod_dm INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/moddisplay_manager_bindings.c
    ${CMAKE_CURRENT_LIST_DIR}/modseedsigner_bindings.c
    ${CMAKE_CURRENT_LIST_DIR}/modsecp256k1_bindings.c
    ${CMAKE_CURRENT_LIST_DIR}/modmweb_bindings.c
    ${CMAKE_CURRENT_LIST_DIR}/modlvgl_bindings.c
    ${CMAKE_CURRENT_LIST_DIR}/modlvglcanvas_bindings.c
    ${CMAKE_CURRENT_LIST_DIR}/modquirc_bindings.c
    ${CMAKE_CURRENT_LIST_DIR}/../deps/seedsigner/src/lvgl/mod_lvgl.c
    ${CMAKE_CURRENT_LIST_DIR}/../deps/seedsigner/src/lvgl/Inconsolata_SemiBold.c
    ${CMAKE_CURRENT_LIST_DIR}/../deps/seedsigner/src/lvgl/Font_Awesome_6_Free_24.c
    ${CMAKE_CURRENT_LIST_DIR}/../deps/seedsigner/src/lvgl/Font_Awesome_6_Free_36.c
    ${SEEDSIGNER_C_MODULES_DIR}/components/seedsigner/fonts/opensans_regular_17_4bpp_125x.c
)

target_include_directories(usermod_dm INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../ports/esp32/display_manager
    ${CMAKE_CURRENT_LIST_DIR}/../ports/esp32/board_common/src
    ${CMAKE_CURRENT_LIST_DIR}/../ports/esp32/board_common/components/esp-camera-pipeline/components/k_quirc/include
    ${CMAKE_CURRENT_LIST_DIR}/../deps/seedsigner/src/lvgl
    ${CMAKE_CURRENT_LIST_DIR}/../deps/secp256k1
    ${SEEDSIGNER_C_MODULES_DIR}/components/seedsigner
)

target_compile_options(usermod_dm INTERFACE
    -mtext-section-literals
)

# Link bindings against ESP-IDF component libs instead of compiling component C++
# sources in usermod qstr extraction.
target_link_libraries(usermod_dm INTERFACE
    __idf_display_manager
    __idf_seedsigner
    __idf_mweb
    __idf_k_quirc
)

target_link_libraries(usermod INTERFACE usermod_dm)
