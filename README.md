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
- Inter-thread communication architecture and application logic based on Zephyr Bus and Message Queues 

### System architecture



### BLE Services

| Service | Service UUID | Characteristic | Characteristic UUID | Properties | Permissions | Descriptors | Description |
|---|---|---|---|---|---|---|---|
| THP Service | `d5d63513-0971-4e4f-abfa-4b37c97f40c3` | Temperature | `d5d63514-0971-4e4f-abfa-4b37c97f40c3` | Notify | None | CUD (0x2901), CPF (0x2904), CCC (0x2902) | Temperature [Celsius] |
| THP Service | `d5d63513-0971-4e4f-abfa-4b37c97f40c3` | Humidity | `d5d63515-0971-4e4f-abfa-4b37c97f40c3` | Notify | None | CUD (0x2901), CPF (0x2904), CCC (0x2902) | Humidity [%] |
| THP Service | `d5d63513-0971-4e4f-abfa-4b37c97f40c3` | Pressure | `d5d63516-0971-4e4f-abfa-4b37c97f40c3` | Notify | None | CUD (0x2901), CPF (0x2904), CCC (0x2902) | Pressure [Pa] |
| THP Service | `d5d63513-0971-4e4f-abfa-4b37c97f40c3` | Sampling Interval | `d5d63517-0971-4e4f-abfa-4b37c97f40c3` | Read, Write | Read, Write Authenticated | CUD (0x2901), CPF (0x2904) | Sampling interval [ms] |
| Nordic LBS | `00001523-1212-efde-1523-785feabcd123` | Button | `00001524-1212-efde-1523-785feabcd123` | Read, Indicate | Read | CCC (0x2902) | Button state |
| Nordic LBS | `00001523-1212-efde-1523-785feabcd123` | LED | `00001525-1212-efde-1523-785feabcd123` | Write | Write Authenticated | — | LED control |
| Battery Service (BAS) | `0x180F` | Battery Level | `0x2A19` | Read, Notify | — | CCC (0x2902), CPF (0x2904) | Battery level [%] |
| SMP Service | `8D53DC1D-1DB7-4CD3-868B-8A527460AA84` | SMP Characteristic | `DA2E7828-FBCE-4E01-AE9E-261174997C48` | Write Without Response, Notify | — | CCC (0x2902) | MCUmgr / DFU |

### User Interface

 - **Button 1** - Button for Nordic LBS
 - **Button 2** - Unpair, clear all bonding information and restart advertising
 - **Button 3** - Enter pairing mode and advertise wihout filter accept list

 - **LED 1** - Activity / current pairing state indication (fast blinking in pairing mode)
 - **LED 2** - Connection indication
 - **LED 3** - LED for Nordic LBS

### External IC's

| nRF52 pin | External IC Signal | Notes |
|---|---|---|
| **BME280** |  |  |
| P0.28 | SCK | SPI1 |
| P0.29 | MOSI (SDI) | SPI1 |
| P0.31 | MISO (SDO) | SPI1 |
| P0.30 | CSB | SPI1, active low |
| VDD | VCC | 3.3V |
| GND | GND | — |
| **MX25R64 Flash** |  |  |
| P0.25 | SCK | SPI2 / nRF7002DK: P0.08 |
| P0.23 | MOSI (SI) | SPI2 / nRF7002DK: P0.09 |
| P0.24 | MISO (SO) | SPI2 / nRF7002DK: P0.10 |
| P0.22 | CS# | SPI2, active low / nRF7002DK: P0.11 |
| VDD | VCC | 3.3V |
| GND | GND | — |

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
