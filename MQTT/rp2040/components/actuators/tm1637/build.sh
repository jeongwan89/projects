#!/bin/bash

# TM1637 테스트 프로그램 빌드 스크립트

# 빌드 디렉토리 생성
mkdir -p build
cd build

# CMake 설정 및 빌드
cmake ..
make -j4

# 빌드 결과 표시
if [ $? -eq 0 ]; then
    echo ""
    echo "========================================="
    echo "빌드 성공!"
    echo "========================================="
    echo ""
    ls -lh tm1637_test.uf2 tm1637_test.elf 2>/dev/null
    echo ""
    echo "업로드 방법:"
    echo "  1. RP2040의 BOOTSEL 버튼을 누른 채로 USB 연결"
    echo "  2. 다음 명령 실행:"
    echo "     cp build/tm1637_test.uf2 /media/\$USER/RPI-RP2/"
    echo ""
else
    echo ""
    echo "========================================="
    echo "빌드 실패!"
    echo "========================================="
    exit 1
fi
