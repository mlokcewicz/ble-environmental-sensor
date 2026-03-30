### BLE Services

| Service | Service UUID | Characteristic | Characteristic UUID | Properties | Permissions | Descriptors | Description |
|---|---|---|---|---|---|---|---|
| THP Service | `d5d63513-0971-4e4f-abfa-4b37c97f40c3` | Temperature | `d5d63514-0971-4e4f-abfa-4b37c97f40c3` | Notify | None | CUD (0x2901), CCC (0x2902) | Temperature [Celsius] |
| THP Service | `d5d63513-0971-4e4f-abfa-4b37c97f40c3` | Humidity | `d5d63515-0971-4e4f-abfa-4b37c97f40c3` | Notify | None | CUD (0x2901), CCC (0x2902) | Humidity [%] |
| THP Service | `d5d63513-0971-4e4f-abfa-4b37c97f40c3` | Pressure | `d5d63516-0971-4e4f-abfa-4b37c97f40c3` | Notify | None | CUD (0x2901), CCC (0x2902) | Pressure [hPa] |
| THP Service | `d5d63513-0971-4e4f-abfa-4b37c97f40c3` | Sampling Interval | `d5d63517-0971-4e4f-abfa-4b37c97f40c3` | Read, Write | Read, Write Authenticated | — | Sampling interval [ms] |
| Nordic LBS | `00001523-1212-efde-1523-785feabcd123` | Button | `00001524-1212-efde-1523-785feabcd123` | Read, Indicate | Read | CCC (0x2902) | Button state |
| Nordic LBS | `00001523-1212-efde-1523-785feabcd123` | LED | `00001525-1212-efde-1523-785feabcd123` | Write | Write Authenticated | — | LED control |
| Battery Service (BAS) | `0x180F` | Battery Level | `0x2A19` | Read, Notify | — | CCC (0x2902), CPF (0x2904) | Battery level [%] |
| SMP Service | `8D53DC1D-1DB7-4CD3-868B-8A527460AA84` | SMP Characteristic | `DA2E7828-FBCE-4E01-AE9E-261174997C48` | Write Without Response, Notify | — | CCC (0x2902) | MCUmgr / DFU |