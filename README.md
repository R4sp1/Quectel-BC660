# Arduino library for Quectel BC660K-GL NB-IoT module

Library is based on [M2M_Quectel libary from M2M solutions AB](https://github.com/m2m-solutions/M2M_Quectel) and partialy on [AT-command-library from Timothy Woo](https://github.com/botletics/AT-Command-Library).
Library is using UART to send AT commands to Quectel module. Some logic is implemented to.

## Current state
- Implemeted function's to get comunication with module started and to get some data back.
- Only HardwareSerial is supported for now.