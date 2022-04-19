# Communication
The boom uses RS485 at 1M baud to send data to the Speedgoat.
Data frames are sent at 1kHz.

## Connection
| Signal | Wire | IO393 pin |
| ------- | ------- | ------- |
| 5V | Red | 2b |
| 0V | Blue | 1b |
| A (+) | Green | 5a
| B (-) | Yellow | 6a

|                | bytes | type  |
| -------------- | ----- | ----- |
| header         | 16    |       |
| x position     | 32    | float |
| y position     | 32    | float |
| ϕ position     | 32    | float |
| x velocity     | 32    | float |
| y velocity     | 32    | float |
| ϕ velocity     | 32    | float |
| x acceleration | 32    | float |
| y acceleration | 32    | float |

# Sensors

## Pitch & Yaw Axis Encoders
[Omron E6B2-CWZ6C Incremental 1024ppr](https://www.ia.omron.com/data_pdf/cat/e6b2-c_ds_e_6_1_csm491.pdf?id=487)

The pitch axis requires indexing at startup.
The yaw axis is zeroed at startup.

| Signal | Wire | Teensy pin |
| ------- | ------- | ------- |
| 5V | Brown | - |
| 0V | Blue | - |
| A | Black | 0 & 6
| B | White | 1 & 7
| Z | Orange | 2 & 8

## Roll Axis Encoder (Optional)
[Avago HEDS-5540#A06 Incremental 500ppr](https://docs.broadcom.com/doc/AV02-1046EN)

The roll axis encoder is optional. It requires indexing at startup if it is connected. If it is disconnected the Teensy will just send its constant index offset value.

| Signal | Wire | Teensy pin |
| ------- | ------- | ------- |
| 5V | Red | - |
| 0V | Black | - |
| A | Green | 10 |
| B | Blue | 11 |
| Z | Yellow | 12|

## IMU
[ST LSM9DS1 IMU](https://www.st.com/resource/en/datasheet/lsm9ds1.pdf)

| Signal | Wire | Teensy pin |
| ------- | ------- | ------- |
| 3.3V | Red | - |
| 0V | Blue | - |
| SDA | Green | 18 |
| SCL | Yellow | 19 |
