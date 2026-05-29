# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13)

# Initialize the Pico SDK
include(pico_sdk_import.cmake)

project(SecureNode C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Define your executable
add_executable(secure_node main.c)

# Pull in the hardware libraries you need
target_link_libraries(secure_node pico_stdlib hardware_adc)

# Enable USB serial output (Disables UART serial)
pico_enable_stdio_usb(secure_node 1)
pico_enable_stdio_uart(secure_node 0)

# Create the .uf2 file for dragging and dropping
pico_add_extra_outputs(secure_node)