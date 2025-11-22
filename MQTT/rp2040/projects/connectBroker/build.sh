#!/bin/bash

# RP2040 MQTT 프로젝트 빌드 스크립트

# 색상 정의
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== RP2040 MQTT 프로젝트 빌드 ===${NC}"

# PICO_SDK_PATH 확인
if [ -z "$PICO_SDK_PATH" ]; then
    echo -e "${RED}오류: PICO_SDK_PATH 환경변수가 설정되지 않았습니다.${NC}"
    echo -e "${YELLOW}다음 명령으로 설정하세요:${NC}"
    echo "export PICO_SDK_PATH=/path/to/pico-sdk"
    exit 1
fi

echo -e "${GREEN}PICO SDK 경로: $PICO_SDK_PATH${NC}"

# 빌드 디렉토리 생성
if [ ! -d "build" ]; then
    echo -e "${YELLOW}빌드 디렉토리 생성...${NC}"
    mkdir build
fi

cd build

# CMake 설정
echo -e "${YELLOW}CMake 설정...${NC}"
cmake .. -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo -e "${RED}CMake 설정 실패${NC}"
    exit 1
fi

# 빌드
echo -e "${YELLOW}컴파일 중...${NC}"
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo -e "${RED}빌드 실패${NC}"
    exit 1
fi

echo -e "${GREEN}=== 빌드 완료 ===${NC}"
echo -e "${GREEN}생성된 파일:${NC}"
ls -lh *.uf2 *.elf 2>/dev/null || echo "UF2/ELF 파일을 찾을 수 없습니다."

echo ""
echo -e "${YELLOW}RP2040에 업로드하는 방법:${NC}"
echo "1. RP2040의 BOOTSEL 버튼을 누른 상태로 USB 연결"
echo "2. RPI-RP2 드라이브가 마운트되면 .uf2 파일을 복사"
echo "   cp build/rp2040_mqtt_client.uf2 /media/RPI-RP2/"
