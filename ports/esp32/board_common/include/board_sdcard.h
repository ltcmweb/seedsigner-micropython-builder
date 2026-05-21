#pragma once

/** Initialise SD card via SDMMC. Pin mapping from board_config.h. */
void board_sdcard_init(void);

/** Get SD card size in bytes (0 if not mounted). */
uint64_t board_sdcard_get_size(void);
