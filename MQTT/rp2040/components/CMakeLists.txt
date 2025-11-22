# CMake 최소 버전 및 프로젝트 명시 (최상단)
cmake_minimum_required(VERSION 3.18)
cmake_policy(SET CMP0057 NEW)
project(rp2040_components)

# Pico SDK 필수 매크로 및 보드 설정 (단독 빌드 대응)
if(NOT DEFINED PICO_BOARD)
	set(PICO_BOARD pico)
endif()
if(NOT DEFINED PICO_PLATFORM)
	set(PICO_PLATFORM rp2040)
endif()
add_definitions(-DPICO_BOARD="${PICO_BOARD}")
add_definitions(-DPICO_PLATFORM="${PICO_PLATFORM}")
# 상위 빌드에서 생성된 config_autogen.h를 복사
set(UPPER_CONFIG_AUTOGEN_PATH "${CMAKE_SOURCE_DIR}/../projects/connectBroker/build/generated/pico_base/pico/config_autogen.h")
set(COMPONENTS_CONFIG_AUTOGEN_PATH "${CMAKE_BINARY_DIR}/generated/pico_base/pico/config_autogen.h")
add_custom_command(
	OUTPUT ${COMPONENTS_CONFIG_AUTOGEN_PATH}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${UPPER_CONFIG_AUTOGEN_PATH} ${COMPONENTS_CONFIG_AUTOGEN_PATH}
	DEPENDS ${UPPER_CONFIG_AUTOGEN_PATH}
	COMMENT "Copying config_autogen.h from upper build."
)
add_custom_target(copy_config_autogen ALL DEPENDS ${COMPONENTS_CONFIG_AUTOGEN_PATH})
# version.h 자동 생성 (상위 프로젝트와 동일하게)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/generated/pico_base/pico)
configure_file(
	"$ENV{PICO_SDK_PATH}/src/common/pico_base_headers/include/pico/version.h.in"
	${CMAKE_BINARY_DIR}/generated/pico_base/pico/version.h
)
include_directories(BEFORE
	${CMAKE_BINARY_DIR}/generated
	${CMAKE_BINARY_DIR}/generated/pico_base
	${CMAKE_BINARY_DIR}/generated/pico_base/pico  # config_autogen.h 포함
	${PICO_SDK_PATH}/src/rp2040/pico_platform/include
	${PICO_SDK_PATH}/src/rp2_common/pico_platform_compiler/include
	${PICO_SDK_PATH}/src/common/pico_base_headers/include
	${PICO_SDK_PATH}/src/common/pico_stdlib_headers/include
	${PICO_SDK_PATH}/src/boards/include
	${PICO_SDK_PATH}/src/rp2040/hardware_regs/include
	${PICO_SDK_PATH}/src/rp2_common/pico_platform_sections/include
	${PICO_SDK_PATH}/src/rp2_common/pico_platform_panic/include
	${CMAKE_SOURCE_DIR}/projects/connectBroker/inc
)

# Pico SDK import 및 초기화 (단독 빌드 대응)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../projects/connectBroker/pico_sdk_import.cmake")
	include("${CMAKE_CURRENT_SOURCE_DIR}/../projects/connectBroker/pico_sdk_import.cmake")
elseif(DEFINED ENV{PICO_SDK_PATH})
	include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
else()
	message(FATAL_ERROR "Pico SDK를 찾을 수 없습니다. 환경변수 PICO_SDK_PATH를 설정하거나 projects/connectBroker에 pico_sdk_import.cmake를 두세요.")
endif()
pico_sdk_init()

# Pico SDK 헤더 경로 명시적으로 추가 (pico/stdlib.h 등)
include_directories(BEFORE
	${PICO_SDK_PATH}/src/common/pico_stdlib_headers/include
	${PICO_SDK_PATH}/src/common/pico_base_headers/include
	${PICO_SDK_PATH}/src/rp2_common/hardware_gpio/include
	${PICO_SDK_PATH}/src/rp2_common/hardware_uart/include
	${PICO_SDK_PATH}/src/rp2_common/hardware_adc/include
	${PICO_SDK_PATH}/src/boards/include
)
# components/CMakeLists.txt
add_subdirectory(actuators)
add_subdirectory(sensors)
add_subdirectory(wifi_mqtt)
