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