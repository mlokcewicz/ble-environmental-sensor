# ble-environmental-sensor 

BLE environmental sensor (temperature, humidity, pressure) based on Nordic nRF52832 MCU, MX25R64 flash and BME280 sensor.

## Capabilities

- Temperature, humidity and pressure measurements sending to central by notifications via custom BLE service 
- Nordic Led Button Service handling for user-defined purposes
- Supply voltage measurement and sending via Battery Service
- DFU over BLE enabled via SMP Service
- Sampling interval configuration
- Pairing with encryption and authentication and bonding with filer accept list handling
- Up to 2 paired devices handling, ability to reset bonded list and enter pairing mode
- DFU over BLE and Serial Recovery in MCUboot (used as immutable bootloader)
- External SPI NOR flash used for the MCUboot's secondary slot

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
