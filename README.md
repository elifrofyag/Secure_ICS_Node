# Secure ICS Node on Raspberry Pi Pico
A simple implementation/prototype to model a secure ICS architecture with readily available resources QEMU-based OP-TEE on Windows host computer and Raspberry Pi Pico (no wifi + no bluetooth) microcontroller. 

![architecture](/assets/telemetry_flow.png)


## 1. set up and run hardware
### wiring
![wiring](/assets/wiring_pico.png)
### install
**NOTE: for simplicity, i have included the secure_node/secure_node.uf2 -> just need to hold down `BOOTSEL` button on Pico, plug it into computer via USB, and release button, then drag and drop the .uf2 file onto it. Pico will automatically reboot and start running C code. then skip the build steps below and move on to the "test" section.**


1. on windows, get VSCode and install extensions "C/C++ Extension Pack" and "Raspberry Pi Pico" and "CMake Tools"; or following this guide: https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf
2. clone this repo and open in VSCode
3. It will ask you to innit Pico Project, and when asked to  select a "Kit", choose the GCC for `arm-none-eabi` option

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

## 2. run gateway (server)
1. open a terminal, navigate to the `gateway` folder, and run `npm install` to install dependencies.
2. run `node index.js` to start the gateway server

## 3. run dashboard (client)
1. open another terminal, navigate to the `dashboard` folder, and run `npm install` to install dependencies.
2. run `npm start` and open `http://localhost:3000` in your browser to view the dashboard

## 4. run QEMU and OP-TEE (secure world)
0. make sure you have QEMU installed and set up to work with OP-TEE (follow this guide: https://optee.readthedocs.io/en/latest/building/devices/qemu.html#qemu-v7) - may take a lot of time to build everything the first time. 

1. open ubuntu terminal
check `echo $(ip route show default | awk '{print $3}')` to get your host IP address, then replace `HOST_IP` in the command below and run in the same terminal:
```bash
    $ nc -vz HOST_IP 5000
```
if you see "succeeded", it means the gateway server is ready to accept TCP connections from QEMU. otherwise, check your firewall settings by Allow an app through Windows Firewall -> add "Node.js JavaScript Runtime" and make sure both Private and Public are checked.
2. run below to forward TCP traffic from QEMU to the gateway server
```bash
    socat TCP-LISTEN:5000,reuseaddr,fork TCP:$(ip route show default | awk '{print $3}'):5000
```
3. open another wsl/ubuntu terminal, run below to build and run optee on qemu
```bash
    ~/optee-qemu/build$ make run-only
```
then 2 terminals will spawn up, one for Secure World (OP-TEE) and one for Normal World (Linux). Should see `(qemu)` on the terminal, key in `c`.

4. in the Normal World terminal, type `root` to login and run
```bash
    optee_secure_ics
```

## tips
- if main project repo is on Windows drive, but QEMU build environment in inside WSL2 filesystem; but you still want to track changes in the secure_world C code in optee with git, then
```bash
    echo 'cp -r /mnt/d/path/to/Secure-ICS-Node/secure_world/* ~/optee-qemu/optee_examples/secure_ics/ && cd ~/optee-qemu/build && make -j4 && make run-only' > ~/run_ics.sh
    chmod +x ~/run_ics.sh
```
then simply run `~/run_ics.sh` every time you make changes to the secure_world code on windows, since it will auto grab latest code from Windows, copy it to optee, build and run it.

# progress so far:
1. wiring
![real wiring](/assets/real_wiring.jpg)
2. pico successfully sending telemetry over usb serial to gateway server
![putty](/assets/putty_serial.png)
3. gateway server successfully receiving telemetry and forwarding to optee on qemu via TCP, then receive reponses back from optee
![gateway](/assets/gateway_terminal.png)
4. optee successfully receiving telemetry from gateway, verify HMAC, signs telemetry, and sending back responses to gateway
![optee](/assets/optee_terminal.png)
# sources
https://pip-assets.raspberrypi.com/categories/609-microcontroller-boards/documents/RP-009085-KB-1-raspberry-pi-pico-c-sdk.pdf

https://pip-assets.raspberrypi.com/categories/610-raspberry-pi-pico/documents/RP-008276-DS-1-getting-started-with-pico.pdf

https://pip-assets.raspberrypi.com/categories/610-raspberry-pi-pico/documents/RP-008307-DS-1-pico-datasheet.pdf