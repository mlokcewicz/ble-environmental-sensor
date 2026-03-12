# ble-environmental-sensor 

BLE environmental sensor (temperature, humidity, pressure) based on Nordic Thingy52.

## Tools

* CMake 3.27.0
* Ninja 1.11.1 
* zephyr-sdk-0.17.4
* JLink v796k

## External libraries

* ncs v3.2.2

## Build

### Generate ninja files

Set: `ZEPHYR_SDK_INSTALL_DIR`, `ZEPHYR_BASE`, `Zephyr_DIR`

`west build --sysbuild --pristine --cmake-only -b custom_plank -d build/build/<hw_version_<config> -- -DBOARD_REVISION=<hw_version> -DBOARD_ROOT=. -DCONF_FILE=prj.conf;prj_<config>.conf`                

### Build project
`west build -d build/<hw_version_<config>`
