#!/bin/bash
# deploy.sh

cd /home/kjw/Git/projects/MQTT/rp2040/projects/displayTM1637
rm -rf build
./build.sh

if [ $? -eq 0 ]; then
    echo "✅ 빌드 성공!"
    echo "UF2 파일: build/rp2040_display_tm1637.uf2"
    echo "크기: $(du -h build/rp2040_display_tm1637.uf2 | cut -f1)"
else
    echo "❌ 빌드 실패!"
    exit 1
fi