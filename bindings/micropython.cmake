add_library(usermod_dm INTERFACE)

target_sources(usermod_dm INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/modsecp256k1_bindings.c
    ${CMAKE_CURRENT_LIST_DIR}/modmweb_bindings.c
    ${CMAKE_CURRENT_LIST_DIR}/modcamera_bindings.c
    ${CMAKE_CURRENT_LIST_DIR}/modquirc_bindings.c
    ${CMAKE_CURRENT_LIST_DIR}/../lvgl/mod_lvgl.c
    ${CMAKE_CURRENT_LIST_DIR}/../lvgl/modlvgl_bindings.c
    ${CMAKE_CURRENT_LIST_DIR}/../lvgl/modlvglcanvas_bindings.c
    ${CMAKE_CURRENT_LIST_DIR}/../lvgl/fonts/opensans_regular_17_4bpp.c
    ${CMAKE_CURRENT_LIST_DIR}/../lvgl/fonts/opensans_regular_17_4bpp_125x.c
    ${CMAKE_CURRENT_LIST_DIR}/../lvgl/fonts/opensans_regular_17_4bpp_150x.c
    ${CMAKE_CURRENT_LIST_DIR}/../lvgl/fonts/opensans_regular_17_4bpp_200x.c
    ${CMAKE_CURRENT_LIST_DIR}/../lvgl/fonts/opensans_semibold_18_4bpp.c
    ${CMAKE_CURRENT_LIST_DIR}/../lvgl/fonts/opensans_semibold_20_4bpp.c
    ${CMAKE_CURRENT_LIST_DIR}/../lvgl/fonts/opensans_semibold_26_4bpp.c
    ${CMAKE_CURRENT_LIST_DIR}/../lvgl/fonts/Inconsolata_Regular.c
    ${CMAKE_CURRENT_LIST_DIR}/../lvgl/fonts/Inconsolata_SemiBold.c
    ${CMAKE_CURRENT_LIST_DIR}/../lvgl/fonts/seedsigner_icons_24_4bpp.c
    ${CMAKE_CURRENT_LIST_DIR}/../lvgl/fonts/seedsigner_icons_36_4bpp.c
    ${CMAKE_CURRENT_LIST_DIR}/../lvgl/fonts/seedsigner_icons_48_4bpp.c
    ${CMAKE_CURRENT_LIST_DIR}/../lvgl/fonts/Font_Awesome_6_Free_24.c
    ${CMAKE_CURRENT_LIST_DIR}/../lvgl/fonts/Font_Awesome_6_Free_36.c
    ${CMAKE_CURRENT_LIST_DIR}/../uhashlib/crypto/ripemd160.c
    ${CMAKE_CURRENT_LIST_DIR}/../uhashlib/crypto/sha2.c
    ${CMAKE_CURRENT_LIST_DIR}/../uhashlib/crypto/hmac.c
    ${CMAKE_CURRENT_LIST_DIR}/../uhashlib/crypto/pbkdf2.c
    ${CMAKE_CURRENT_LIST_DIR}/../uhashlib/crypto/memzero.c
    ${CMAKE_CURRENT_LIST_DIR}/../uhashlib/hashlib.c
    ${CMAKE_CURRENT_LIST_DIR}/../uhashlib/uhmac.c
    ${CMAKE_CURRENT_LIST_DIR}/../qrcode/qrcodegen/qrcodegen.c
    ${CMAKE_CURRENT_LIST_DIR}/../qrcode/qrcode.c
)

target_include_directories(usermod_dm INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/../ports/esp32/board_common/src
    ${CMAKE_CURRENT_LIST_DIR}/../ports/esp32/board_common/components/k_quirc/include
    ${CMAKE_CURRENT_LIST_DIR}/../deps/secp256k1
    ${BOARD_CONFIG_DIR}
)

target_compile_options(usermod_dm INTERFACE
    -mtext-section-literals
)

# Link bindings against ESP-IDF component libs instead of compiling component C++
# sources in usermod qstr extraction.
target_link_libraries(usermod_dm INTERFACE
    __idf_mweb
    __idf_k_quirc
)

target_link_libraries(usermod INTERFACE usermod_dm)
