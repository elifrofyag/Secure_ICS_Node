# Secure ICS Node on Raspberry Pi Pico
A simple implementation of a secure ICS node using the Raspberry Pi Pico (no wifi + no bluetooth) microcontroller. The node reads soil moisture data from an ADC, sends telemetry data over USB Serial, and listens for commands to control an LED.

## 1. set up and run hardware
### wiring

### install
1. on windows, get VSCode and install extensions "C/C++ Extension Pack" and "Raspberry Pi Pico" and "CMake Tools"; or following this guide: https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf
2. clone this repo and open in VSCode
3. If you have the CMake Tools extension installed, it will ask you to select a "Kit" (choose the GCC for arm-none-eabi option)

### configure
Generate the compile_commands.json file
If you haven't successfully run a CMake configuration yet, the file doesn't exist. Let's force CMake to create it.

1. Open vs code Command Palette (`Ctrl + Shift + P`). type "CMake: Configure" and hit Enter.
2. Look inside your `build` folder, should now see a file named compile_commands.json.

### compile and flash
1. Click Compile button at the bottom of VS Code.

2. If successful, look inside the newly generated `build` , find a file named `secure_node.uf2`.

3. Hold down `BOOTSEL` button on your Pico, plug it into your computer via USB, and release the button.

4. Drag and drop the .uf2 file onto it. Pico will automatically reboot and start running C code.

### test
open a Serial Monitor (like PuTTY, or the one built into VS Code) connected to the Pico's COM port -> should see the JSON telemetry streaming in, and typing `led_on` with appropriate line ending will trigger LED.

## run gateway



# sources
https://pip-assets.raspberrypi.com/categories/609-microcontroller-boards/documents/RP-009085-KB-1-raspberry-pi-pico-c-sdk.pdf

https://pip-assets.raspberrypi.com/categories/610-raspberry-pi-pico/documents/RP-008276-DS-1-getting-started-with-pico.pdf

https://pip-assets.raspberrypi.com/categories/610-raspberry-pi-pico/documents/RP-008307-DS-1-pico-datasheet.pdf