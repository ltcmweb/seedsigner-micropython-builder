#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
MP_DIR="$ROOT_DIR/deps/micropython/upstream"

BOARD="${BOARD:-WAVESHARE_ESP32_S3_TOUCH_LCD_35B}"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build/$BOARD}"
mkdir -p "$BUILD_DIR"

PORTS_ESP32_DIR="$ROOT_DIR/ports/esp32"
USER_C_MODULES_FILE="$ROOT_DIR/usercmodule.cmake"
MICROPY_CMAKE_ARGS="${CMAKE_ARGS:-} -DUSER_C_MODULES=$USER_C_MODULES_FILE"
CFLAGS_EXTRA="-I$PORTS_ESP32_DIR/include -include $PORTS_ESP32_DIR/include/defs.h"
MICROPY_CMAKE_ARGS="$MICROPY_CMAKE_ARGS -DMICROPY_EXTRA_COMPONENT_DIRS=${PORTS_ESP32_DIR} -DCMAKE_C_FLAGS=\"$CFLAGS_EXTRA\""

# board_common board config: maps MicroPython board name to board_common board dir.
# Override with BOARD_CONFIG_DIR env var, or auto-map from BOARD name.
BOARD_COMMON_DIR="$PORTS_ESP32_DIR/board_common"
if [ -z "${BOARD_CONFIG_DIR:-}" ]; then
  case "$BOARD" in
    WAVESHARE_ESP32_S3_TOUCH_LCD_35B) BOARD_CONFIG_DIR="$BOARD_COMMON_DIR/boards/waveshare_s3_lcd35b" ;;
    WAVESHARE_ESP32_S3_TOUCH_LCD_35)  BOARD_CONFIG_DIR="$BOARD_COMMON_DIR/boards/waveshare_s3_lcd35" ;;
    WAVESHARE_ESP32_P4_WIFI6_TOUCH_LCD_35) BOARD_CONFIG_DIR="$BOARD_COMMON_DIR/boards/waveshare_p4_lcd35" ;;
    WAVESHARE_ESP32_P4_WIFI6_TOUCH_LCD_43) BOARD_CONFIG_DIR="$BOARD_COMMON_DIR/boards/waveshare_p4_lcd43" ;;
    *) echo "WARNING: No board_common mapping for BOARD=$BOARD"; BOARD_CONFIG_DIR="" ;;
  esac
fi
if [ -n "$BOARD_CONFIG_DIR" ] && [ -d "$BOARD_CONFIG_DIR" ]; then
  MICROPY_CMAKE_ARGS="$MICROPY_CMAKE_ARGS -DBOARD_CONFIG_DIR=$BOARD_CONFIG_DIR"
else
  echo "WARNING: BOARD_CONFIG_DIR not found: ${BOARD_CONFIG_DIR:-<unset>}"
fi

make -C "$MP_DIR/mpy-cross" USER_C_MODULES= -j"$(nproc)"
make -C "$MP_DIR/ports/esp32" -j"$(nproc)" \
  BOARD="$BOARD" \
  BUILD="$BUILD_DIR" \
  USER_C_MODULES="$USER_C_MODULES_FILE" \
  CMAKE_ARGS="$MICROPY_CMAKE_ARGS" \
  CFLAGS_EXTRA="$CFLAGS_EXTRA" \
  MICROPY_MPYCROSS="$MP_DIR/mpy-cross/build/mpy-cross" \
  IDF_CCACHE_ENABLE=1 ${TARGET:-all}
