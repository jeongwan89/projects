#!/bin/bash
# components/build.sh
# components 디렉토리 이하 모든 모듈을 빌드하는 스크립트

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake ..
make -j$(nproc)

echo "[INFO] components 빌드 완료: $BUILD_DIR"
