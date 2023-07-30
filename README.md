# MQTT client for the Raspberry Pi Pico
Set up raspberry pi pico projects according the official [documentation](https://projects.raspberrypi.org/en/projects/getting-started-with-the-pico).

Clone this project:
```
git clone git@github.com:MKesenheimer/pico-mqtt.git
```

Copy and modify the file `crypto_consts_example.h`:
```
cp crypto_consts_example.h crypto_consts.h
```
Populate the variables with desired values.
Update the variable `SENSOR_LOCATION` in `main.cpp` to your liking.

Build and flash the project:
```
mkdir build && cd build
cmake ..
make -j4
make flash
```

Test your setup by setting up a MQTT subscriber:
```
mosquitto_sub -h 192.168.2.28 -p 1883 -u <CLIENT_USER> -P <CLIENT_PASSWORD> -t '/home/office3/tvoc'
```

##

## Flash with Pico Probe
```
sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program step-counter.elf verify reset exit"
```

## Debug with Pico Probe
On macOS debugging arm architectures requires the `arm-none-eabi-gdb` debugger.
Install it with
```
sudo port install arm-none-eabi-gdb
```

After flashing, start the gdb proxy:
```
sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000"
```
Or use the makefile command:
```
make debug
```

Then connect with `arm-none-eabi-gdb` and start debugging:
```
$ /opt/local/bin/arm-none-eabi-gdb step-counter.elf
> target extended-remote localhost:3333
> monitor reset init
> continue
```

Debugging with `lldb` does not work right now. In principle one could connect to the gdb proxy via:
```
$ lldb step-counter.elf
(lldb) gdb-remote 3333
(lldb) process plugin packet monitor reset
(lldb) continue
```