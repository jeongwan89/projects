#!/bin/bash
# deploy.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

UF2="build/rp2040_display_tm1637.uf2"

# 빌드
./build.sh

echo ""
echo "UF2 파일: $UF2"
echo "크기: $(du -h "$UF2" | cut -f1)"

# picotool로 업로드 (BOOTSEL 모드 자동 진입 후 플래시)
echo ""
echo "📡 picotool로 업로드 중..."
picotool load "$UF2" --force
picotool reboot

echo "✅ 업로드 완료!"