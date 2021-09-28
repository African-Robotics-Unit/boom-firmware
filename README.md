# boom-logger



|                | bytes | type  |
| -------------- | ----- | ----- |
| header         | 16    |       |
| x position     | 32    | float |
| y position     | 32    | float |
| x velocity     | 32    | float |
| y velocity     | 32    | float |
| x acceleration | 32    | float |
| y acceleration | 32    | float |



Data frame

| 1 - 2 [2] | 3 - 6 [4] | 7 - 10 [4] | 9 - 12 [4] | 13 -16 [4] | 17 - 20 [4] | 21 - 24 [4] |
| :-------: | :-------: | ---------- | ---------- | ---------- | ----------- | ----------- |
| 0xAA 0x55 |   x pos   | y pos      | x vel      | y vel      | x acc       | y acc       |

