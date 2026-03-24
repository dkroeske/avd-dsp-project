# =============================================================================
# PICO DSP PROJECT – met CMSIS-DSP via add_subdirectory
# =============================================================================

cmake_minimum_required(VERSION 3.13)

# Board: pico2 voor RP2350, pico voor RP2040
set(PICO_BOARD pico2)

# Laad Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(dsp_project C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

if(PICO_SDK_VERSION_STRING VERSION_LESS "2.0.0")
    message(FATAL_ERROR "Pico SDK versie 2.0.0 of hoger vereist")
endif()

# -----------------------------------------------------------------------------
# If you want debug output from USB (pass -DPICO_STDIO_USB=1) this ensures y
# -----------------------------------------------------------------------------
if (NOT DEFINED PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS)
    set(PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS 3000)
endif()


# -----------------------------------------------------------------------------
# Pad naar CMSIS-DSP en CMSIS-CORE (git submodule)
# -----------------------------------------------------------------------------
set(CMSIS_DSP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmsis-dsp)

if(NOT EXISTS "${CMSIS_DSP_DIR}/Source/CMakeLists.txt")
    message(FATAL_ERROR 
        "CMSIS-DSP map niet gevonden of incompleet\n"
        "Voer uit:\n"
        "  git submodule add https://github.com/ARM-software/CMSIS-DSP.git cmsis-dsp\n"
        "  git submodule add https://github.com/ARM-software/CMSIS_5.git cmsis_5\n"
        "  git submodule update --init --recursive"
    )
endif()

# Bouw CMSIS-DSP als library
add_subdirectory(${CMSIS_DSP_DIR}/Source cmsisdsp_bin)

# -----------------------------------------------------------------------------
# Applicatie code
# -----------------------------------------------------------------------------
add_executable(${PROJECT_NAME}
    main.c
    process.c
    conv_direct.c 
    # voeg hier eventueel meer bronbestanden toe
)

# -----------------------------------------------------------------------------
# Includes – cruciaal om cmsis_compiler.h te vinden
# -----------------------------------------------------------------------------
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMSIS_DSP_DIR}/Include
    ${CMSIS_DSP_DIR}/PrivateInclude
)

target_include_directories(CMSISDSP PRIVATE
    ${CMSIS_DSP_DIR}/Include
    ${CMSIS_DSP_DIR}/PrivateInclude
)

# CMSIS-Core 
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_5/CMSIS/Core/Include
)
target_include_directories(CMSISDSP PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_5/CMSIS/Core/Include
)

# -----------------------------------------------------------------------------
# Core-definities (kies er één afhankelijk van de gebruikte Pico)
# -----------------------------------------------------------------------------
# Voor Pico 2 (RP2350 – Cortex-M33)
target_compile_definitions(${PROJECT_NAME} PRIVATE ARM_MATH_CM33)
target_compile_definitions(CMSISDSP    PRIVATE ARM_MATH_CM33)

# Voor Pico 1 (RP2040 – Cortex-M0+) → uncomment en verwijder CM33 regels
#target_compile_definitions(${PROJECT_NAME} PRIVATE ARM_MATH_CM0PLUS)
#target_compile_definitions(cmsisdsp    PRIVATE ARM_MATH_CM0PLUS)

# -----------------------------------------------------------------------------
# Link libraries
# -----------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE
    CMSISDSP
    pico_stdlib
    pico_stdio_usb
    hardware_adc
    hardware_dma
    hardware_gpio
    hardware_pio
    hardware_clocks
    hardware_sync
    hardware_irq
)

# -----------------------------------------------------------------------------
# PIO header (voor R-2R DAC)
# -----------------------------------------------------------------------------
pico_generate_pio_header(${PROJECT_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/dac_r2r.pio
)

# -----------------------------------------------------------------------------
# USB / UART instellingen
# -----------------------------------------------------------------------------
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# -----------------------------------------------------------------------------
# Extra outputs (.uf2, .bin, disassembly, etc.)
# -----------------------------------------------------------------------------
pico_add_extra_outputs(${PROJECT_NAME})

# -----------------------------------------------------------------------------
# Compile opties
# -----------------------------------------------------------------------------
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Werror
    -Wextra
    -O0                 # debug – later -O2 of -Ofast
    -g
)

# Optioneel: maak CMSIS-DSP zelf sneller
target_compile_options(CMSISDSP PRIVATE
    -Wsign-compare
    -Wdouble-promotion
    -Ofast
    -ffast-math
    -DNDEBUG
    -Wall
    -Wextra
    -Werror
    -fshort-enums
)
