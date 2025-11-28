# projects
2025년부터 새롭게 정리된 프로젝트를 시작한다. 폴더 관리가 필수가 된다.

## 임베디드 프로젝트 구조

이 저장소는 임베디드 프로젝트를 체계적으로 관리하기 위한 구조를 제공합니다.

### 폴더 구조 원칙

```
프로젝트명/
├── mcu1/          # 첫 번째 MCU용 구현
├── mcu2/          # 두 번째 MCU용 구현
└── mcu3/          # 세 번째 MCU용 구현
```

각 프로젝트는 다음과 같은 계층 구조를 따릅니다:
1. **프로젝트명**: 최상위 폴더로 프로젝트의 기능이나 목적을 나타냅니다
2. **MCU별 폴더**: 각 MCU(마이크로컨트롤러)별로 구현된 코드를 포함합니다

### 예제 프로젝트

#### blink_led
LED를 깜빡이는 기본 예제 프로젝트입니다.

- **stm32/**: STM32 마이크로컨트롤러용 구현 (HAL 라이브러리 사용)
- **arduino/**: Arduino 보드용 구현 (Arduino IDE)
- **esp32/**: ESP32 마이크로컨트롤러용 구현 (ESP-IDF 또는 Arduino)

각 MCU 폴더에는 다음이 포함됩니다:
- 소스 코드 파일
- README.md (빌드 및 실행 방법)
- 필요한 설정 파일

### 새 프로젝트 시작하기

1. 프로젝트 이름으로 폴더 생성
   ```bash
   mkdir my_project
   ```

2. 사용할 MCU별 하위 폴더 생성
   ```bash
   cd my_project
   mkdir stm32 arduino esp32
   ```

3. 각 MCU 폴더에 코드 및 README.md 작성

4. 프로젝트 최상위에 README.md 작성하여 프로젝트 설명 추가

### 지원 MCU 플랫폼

- **STM32**: STM32CubeIDE, HAL 라이브러리
- **Arduino**: Arduino IDE, AVR/ARM 기반 보드
- **ESP32**: ESP-IDF, Arduino Framework

### 기여 방법

새로운 프로젝트나 MCU 플랫폼 추가를 환영합니다. 위의 폴더 구조 원칙을 따라주세요.
