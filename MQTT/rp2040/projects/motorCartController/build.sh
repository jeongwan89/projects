#!/usr/bin/env bash
set -e

# Usage:
#   ./build.sh            # configure + build
#   ./build.sh clean      # remove build directory
#   ./build.sh rebuild    # clean + configure + build
#   ./build.sh upload     # upload existing UF2 to RP2040
#   ./build.sh upload /dev/ttyACM1  # upload using explicit serial device
#   ./build.sh flash      # build + upload to RP2040
#   ./build.sh flash /dev/ttyACM1   # build + upload using explicit serial device

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"
SERIAL_DEV="${2:-}"

if [[ "${PICO_SDK_PATH:-}" == "" ]]; then
  if [[ -d "/home/jeongwan/pico-sdk" ]]; then
    export PICO_SDK_PATH="/home/jeongwan/pico-sdk"
  else
    echo "[ERROR] PICO_SDK_PATH is not set and /home/jeongwan/pico-sdk was not found."
    exit 1
  fi
fi

configure() {
  cmake -S "$ROOT_DIR" -B "$BUILD_DIR" \
    -DPICO_SDK_PATH="$PICO_SDK_PATH" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
}

build() {
  cmake --build "$BUILD_DIR" -j"$(nproc)"
}

resolve_picotool_device_args() {
  local tty_path="$1"
  local tty_name
  local sys_tty
  local dev_dir
  local serial_file

  if [[ -z "$tty_path" ]]; then
    return 0
  fi

  if [[ ! -e "$tty_path" ]]; then
    echo "[WARN] Specified tty not found: $tty_path"
    return 0
  fi

  tty_name="$(basename "$tty_path")"
  sys_tty="/sys/class/tty/$tty_name/device"
  if [[ ! -e "$sys_tty" ]]; then
    echo "[WARN] Could not resolve sysfs path for tty: $tty_path"
    return 0
  fi

  dev_dir="$(readlink -f "$sys_tty")"
  serial_file=""
  if [[ -f "$dev_dir/serial" ]]; then
    serial_file="$dev_dir/serial"
  elif [[ -f "$dev_dir/../serial" ]]; then
    serial_file="$dev_dir/../serial"
  elif [[ -f "$dev_dir/../../serial" ]]; then
    serial_file="$dev_dir/../../serial"
  fi

  if [[ -n "$serial_file" ]]; then
    local usb_ser
    usb_ser="$(cat "$serial_file" 2>/dev/null | tr -d '\n' || true)"
    if [[ -n "$usb_ser" ]]; then
      PICOTOOL_DEVICE_ARGS=(--ser "$usb_ser")
      echo "[INFO] Using USB serial filter from $tty_path: $usb_ser"
      return 0
    fi
  fi

  echo "[WARN] Could not derive USB serial from $tty_path. Using default device selection."
}

upload() {
  local uf2_file="$BUILD_DIR/motorCartController.uf2"
  local tty_dev="$SERIAL_DEV"
  PICOTOOL_DEVICE_ARGS=()

  if [[ ! -f "$uf2_file" ]]; then
    echo "[INFO] UF2 not found. Building first..."
    configure
    build
  fi

  if ! command -v picotool >/dev/null 2>&1; then
    echo "[ERROR] picotool not found. Install picotool first."
    exit 1
  fi

  if [[ -z "$tty_dev" ]]; then
    tty_dev="$(ls /dev/ttyACM* 2>/dev/null | head -n 1 || true)"
  fi

  if [[ -n "$tty_dev" ]]; then
    echo "[INFO] Using USB CDC device: $tty_dev"
    resolve_picotool_device_args "$tty_dev"
    echo "[INFO] Trying to reboot RP2040 into BOOTSEL mode"
    picotool reboot -f -u "${PICOTOOL_DEVICE_ARGS[@]}" || true
  else
    echo "[WARN] No /dev/ttyACM* found. Trying direct picotool upload."
  fi

  echo "[INFO] Uploading UF2: $uf2_file"
  picotool load -f -x "$uf2_file" "${PICOTOOL_DEVICE_ARGS[@]}"
}

case "${1:-build}" in
  clean)
    echo "[INFO] Cleaning build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
    ;;
  rebuild)
    echo "[INFO] Rebuilding project"
    rm -rf "$BUILD_DIR"
    configure
    build
    ;;
  build)
    echo "[INFO] Configuring project"
    configure
    echo "[INFO] Building project"
    build
    ;;
  upload)
    upload
    ;;
  flash)
    echo "[INFO] Configuring project"
    configure
    echo "[INFO] Building project"
    build
    upload
    ;;
  *)
    echo "Usage: $0 [build|clean|rebuild|upload|flash] [/dev/ttyACMx]"
    exit 1
    ;;
esac

if [[ -f "$BUILD_DIR/motorCartController.uf2" ]]; then
  echo "[OK] UF2: $BUILD_DIR/motorCartController.uf2"
fi
if [[ -f "$BUILD_DIR/motorCartController.elf" ]]; then
  echo "[OK] ELF: $BUILD_DIR/motorCartController.elf"
fi
