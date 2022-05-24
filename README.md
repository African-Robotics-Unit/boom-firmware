# Planarizing Boom Firmware

![Build with PlatformIO](https://img.shields.io/badge/build%20with-PlatformIO-orange?logo=data%3Aimage%2Fsvg%2Bxml%3Bbase64%2CPHN2ZyB3aWR0aD0iMjUwMCIgaGVpZ2h0PSIyNTAwIiB2aWV3Qm94PSIwIDAgMjU2IDI1NiIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIiBwcmVzZXJ2ZUFzcGVjdFJhdGlvPSJ4TWlkWU1pZCI+PHBhdGggZD0iTTEyOCAwQzkzLjgxIDAgNjEuNjY2IDEzLjMxNCAzNy40OSAzNy40OSAxMy4zMTQgNjEuNjY2IDAgOTMuODEgMCAxMjhjMCAzNC4xOSAxMy4zMTQgNjYuMzM0IDM3LjQ5IDkwLjUxQzYxLjY2NiAyNDIuNjg2IDkzLjgxIDI1NiAxMjggMjU2YzM0LjE5IDAgNjYuMzM0LTEzLjMxNCA5MC41MS0zNy40OUMyNDIuNjg2IDE5NC4zMzQgMjU2IDE2Mi4xOSAyNTYgMTI4YzAtMzQuMTktMTMuMzE0LTY2LjMzNC0zNy40OS05MC41MUMxOTQuMzM0IDEzLjMxNCAxNjIuMTkgMCAxMjggMCIgZmlsbD0iI0ZGN0YwMCIvPjxwYXRoIGQ9Ik0yNDkuMzg2IDEyOGMwIDY3LjA0LTU0LjM0NyAxMjEuMzg2LTEyMS4zODYgMTIxLjM4NkM2MC45NiAyNDkuMzg2IDYuNjEzIDE5NS4wNCA2LjYxMyAxMjggNi42MTMgNjAuOTYgNjAuOTYgNi42MTQgMTI4IDYuNjE0YzY3LjA0IDAgMTIxLjM4NiA1NC4zNDYgMTIxLjM4NiAxMjEuMzg2IiBmaWxsPSIjRkZGIi8+PHBhdGggZD0iTTE2MC44NjkgNzQuMDYybDUuMTQ1LTE4LjUzN2M1LjI2NC0uNDcgOS4zOTItNC44ODYgOS4zOTItMTAuMjczIDAtNS43LTQuNjItMTAuMzItMTAuMzItMTAuMzJzLTEwLjMyIDQuNjItMTAuMzIgMTAuMzJjMCAzLjc1NSAyLjAxMyA3LjAzIDUuMDEgOC44MzdsLTUuMDUgMTguMTk1Yy0xNC40MzctMy42Ny0yNi42MjUtMy4zOS0yNi42MjUtMy4zOWwtMi4yNTggMS4wMXYxNDAuODcybDIuMjU4Ljc1M2MxMy42MTQgMCA3My4xNzctNDEuMTMzIDczLjMyMy04NS4yNyAwLTMxLjYyNC0yMS4wMjMtNDUuODI1LTQwLjU1NS01Mi4xOTd6TTE0Ni41MyAxNjQuOGMtMTEuNjE3LTE4LjU1Ny02LjcwNi02MS43NTEgMjMuNjQzLTY3LjkyNSA4LjMyLTEuMzMzIDE4LjUwOSA0LjEzNCAyMS41MSAxNi4yNzkgNy41ODIgMjUuNzY2LTM3LjAxNSA2MS44NDUtNDUuMTUzIDUxLjY0NnptMTguMjE2LTM5Ljc1MmE5LjM5OSA5LjM5OSAwIDAgMC05LjM5OSA5LjM5OSA5LjM5OSA5LjM5OSAwIDAgMCA5LjQgOS4zOTkgOS4zOTkgOS4zOTkgMCAwIDAgOS4zOTgtOS40IDkuMzk5IDkuMzk5IDAgMCAwLTkuMzk5LTkuMzk4em0yLjgxIDguNjcyYTIuMzc0IDIuMzc0IDAgMSAxIDAtNC43NDkgMi4zNzQgMi4zNzQgMCAwIDEgMCA0Ljc0OXoiIGZpbGw9IiNFNTcyMDAiLz48cGF0aCBkPSJNMTAxLjM3MSA3Mi43MDlsLTUuMDIzLTE4LjkwMWMyLjg3NC0xLjgzMiA0Ljc4Ni01LjA0IDQuNzg2LTguNzAxIDAtNS43LTQuNjItMTAuMzItMTAuMzItMTAuMzItNS42OTkgMC0xMC4zMTkgNC42Mi0xMC4zMTkgMTAuMzIgMCA1LjY4MiA0LjU5MiAxMC4yODkgMTAuMjY3IDEwLjMxN0w5NS44IDc0LjM3OGMtMTkuNjA5IDYuNTEtNDAuODg1IDIwLjc0Mi00MC44ODUgNTEuODguNDM2IDQ1LjAxIDU5LjU3MiA4NS4yNjcgNzMuMTg2IDg1LjI2N1Y2OC44OTJzLTEyLjI1Mi0uMDYyLTI2LjcyOSAzLjgxN3ptMTAuMzk1IDkyLjA5Yy04LjEzOCAxMC4yLTUyLjczNS0yNS44OC00NS4xNTQtNTEuNjQ1IDMuMDAyLTEyLjE0NSAxMy4xOS0xNy42MTIgMjEuNTExLTE2LjI4IDMwLjM1IDYuMTc1IDM1LjI2IDQ5LjM2OSAyMy42NDMgNjcuOTI2em0tMTguODItMzkuNDZhOS4zOTkgOS4zOTkgMCAwIDAtOS4zOTkgOS4zOTggOS4zOTkgOS4zOTkgMCAwIDAgOS40IDkuNCA5LjM5OSA5LjM5OSAwIDAgMCA5LjM5OC05LjQgOS4zOTkgOS4zOTkgMCAwIDAtOS4zOTktOS4zOTl6bS0yLjgxIDguNjcxYTIuMzc0IDIuMzc0IDAgMSAxIDAtNC43NDggMi4zNzQgMi4zNzQgMCAwIDEgMCA0Ljc0OHoiIGZpbGw9IiNGRjdGMDAiLz48L3N2Zz4=)

This firmware runs on the boom [Teensy 4.0](https://www.pjrc.com/store/teensy40.html) and is responsible for estimating the state of the end of the boom and sending this over RS485 to the control computer. This project has been configured to work with [PlatformIO](https://platformio.org/platformio-ide).

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
| header         | 16    | 0xAA55         |
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

