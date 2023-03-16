# Arduino library for Quectel BC660K-GL NB-IoT module

Library is based on [M2M_Quectel libary from M2M solutions AB](https://github.com/m2m-solutions/M2M_Quectel) and partialy on [AT-command-library from Timothy Woo](https://github.com/botletics/AT-Command-Library).
Library is using UART line to send AT commands to the Quectel module. Some basic logic is implemented too.

## Current state
- Implemeted function's to get comunication with module started and to get some data back.
- Only HardwareSerial is supported for now.
- Basic MQTT communication is supported.
- Basic UDP communication is supported.
