# Planarizing Boom Firmware
This firmware runs on the boom Teensy 4.0 and is responsible for estimating the state of the end of the boom and sending this over RS485 to the control computer.

## Usage
Connect the board to the Speedgoat. The orange LED on the Teensy will come on for about 0.5 seconds and then turn off again. Do not move the boom during this time as this is the IMU being calibrated. Once the LED is off, lift the boom arm and the rotate the pivoting end (only if installed) past their encoder index points. Once the encoders have been indexed the orange LED will remain on. This means the boom is ready and is currently sending state data to the Speedgoat.

## Connections
<p align="center">
<img src="https://github.com/African-Robotics-Unit/boom-firmware/blob/main/boom_board.jpg" width="500">
</p>

## Communication
The boom uses RS485 at 1M baud to send data to the Speedgoat.
Data frames are sent at 1kHz.

### Speedgoat RS485 Connection
| Signal | Wire | IO393 pin |
| ------- | ------- | ------- |
| 5V | Red | 2b |
| 0V | Blue | 1b |
| A (+) | Green | 5a
| B (-) | Yellow | 6a

### Data Frame
| description    | bits | type  |
| -------------- | ----- | ----- |
| header         | 16    |       |
| x position     | 32    | IEEE 754 Float |
| y position     | 32    | IEEE 754 Float |
| ϕ position     | 32    | IEEE 754 Float |
| x velocity     | 32    | IEEE 754 Float |
| y velocity     | 32    | IEEE 754 Float |
| ϕ velocity     | 32    | IEEE 754 Float |
| x acceleration | 32    | IEEE 754 Float |
| y acceleration | 32    | IEEE 754 Float |

## Sensors

### Pitch & Yaw Axis Encoders
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

### Roll Axis Encoder (Optional)
[Avago HEDS-5540#A06 Incremental 500ppr](https://docs.broadcom.com/doc/AV02-1046EN)

The roll axis encoder is optional. It requires indexing at startup if it is connected. If it is disconnected the Teensy will just send its constant index offset value.

The Teensy 4.0 is only 3.3V tollerant so this encoder is powered with 3.3V rather than 5V. It seems to work fine.

| Signal | Wire | Teensy pin |
| ------- | ------- | ------- |
| 5V | Red | - |
| 0V | Black | - |
| A | Green | 10 |
| B | Blue | 11 |
| Z | Yellow | 12|

### IMU
[ST LSM9DS1 IMU](https://www.st.com/resource/en/datasheet/lsm9ds1.pdf)

The IMU is calibrated at startup to ensure the y axis is always oriented vertically even if the pivoting end is not horizontal.

| Signal | Wire | Teensy pin |
| ------- | ------- | ------- |
| 3.3V | Red | - |
| 0V | Blue | - |
| SDA | Green | 18 |
| SCL | Yellow | 19 |


## PLL Velocity Estimator
A PLL is used to compute a more accurate velocity estimate without timers on the signal lines. The PLL runs in an `IntervalTimer` with a priority of 1 to ensure consistent execution at 20kHz.

