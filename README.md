# ble-environmental-sensor 

BLE environmental sensor (temperature, humidity, pressure) based on Nordic Thingy52.

## Capabilities


- temp. humidity, pressure (notifications) 
- led and button
- custom service
- pairing with encryption and authentication, bonding
- filer accept list
- dfu over ble?

## Tools

* CMake 3.27.0
* Ninja 1.11.1 
* zephyr-sdk-0.17.4
* JLink v796k

## External libraries

* ncs v3.2.2

## Build

Application developed as a freestanding Zephyr application type.

### Generate ninja files

Set: `ZEPHYR_SDK_INSTALL_DIR`and `ZEPHYR_BASE`

`west build --sysbuild --pristine --cmake-only -b env_sens_board -d build/build/<hw_version_<config> -- -DBOARD_REVISION=<hw_version> -DBOARD_ROOT=. -DCONF_FILE=prj.conf;prj_<config>.conf`                

### Build project
`west build -d build/<hw_version_<config>`
