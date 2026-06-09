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

if(NOT DEFINED ENV{PICO_SDK_PATH})
    set(ENV{PICO_SDK_PATH} "C:/Users/An/.pico-sdk/sdk/2.2.0")
endif()

# 1. Pull the SDK import file from your edge_node folder
include(edge_node/pico_sdk_import.cmake)

# 2. Define the global monorepo project
project(Secure_ICS C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# 3. Initialize the Pico SDK at the top level
pico_sdk_init()

# 4. Tell CMake to look inside the edge_node folder for the actual code
add_subdirectory(edge_node)
