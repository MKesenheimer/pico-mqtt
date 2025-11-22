cmake_minimum_required(VERSION 3.12)

set(PICO_BOARD pico_w)
set(PICO_SDK_FETCH_FROM_GIT on)
include(pico_sdk_import.cmake)

project(mqtt-test
        LANGUAGES ASM C CXX
        VERSION 1.0
        DESCRIPTION "MQTT Test"
)

# set(CMAKE_C_STANDARD 11)
# set(CMAKE_CXX_STANDARD 19)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

set(FILE_ELF ${CMAKE_PROJECT_NAME}.elf)
set(FILE_HEX ${CMAKE_PROJECT_NAME}.hex)

# Initialize the SDK
pico_sdk_init()
add_executable(${CMAKE_PROJECT_NAME} main.cpp)

target_sources(${CMAKE_PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/pico_mqtt.c
)

# enable usb output, disable uart output
#pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 1)
#pico_enable_stdio_uart(${CMAKE_PROJECT_NAME} 0)

target_compile_definitions(${CMAKE_PROJECT_NAME}
    PRIVATE
        NO_SYS=1
)

target_include_directories(${CMAKE_PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${CMAKE_PROJECT_NAME}
  pico_stdlib
  hardware_i2c
  pico_cyw43_arch_lwip_poll
  pico_lwip_mbedtls
  pico_lwip_mqtt
  pico_mbedtls
)

pico_add_extra_outputs(${CMAKE_PROJECT_NAME})


# add flash target (make flash)
add_custom_target(flash DEPENDS ${PROJECT_NAME}.elf)
add_custom_command(TARGET flash
    USES_TERMINAL
    COMMAND sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program ${PROJECT_NAME}.elf verify reset exit")

# add debug target (make debug)
#set(CMAKE_C_GDB ${GCC_PATH}/arm-none-eabi-gdb CACHE PATH "" FORCE)
add_custom_target(debug DEPENDS ${PROJECT_NAME}.elf)
add_custom_command(TARGET debug
    USES_TERMINAL
    COMMAND xterm -e sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" &)