"""
Class to read and log data from the teensy connected to the boom sensors.
"""

from typing import List, NamedTuple
import csv
import serial
from serial.tools.list_ports import comports
import struct
import time
from threading import Thread


class BoomData(NamedTuple):
    time: float
    yaw: int
    pitch: int
    x: float
    y: float
    dx: float
    dy: float
    ddx: float
    ddy: float
    ddz: float
    temp: int


HEADER = bytes([0xAA, 0x55])
DATAFMT = '<2i7fi'


SENSOR_PARAMS = {
    'baudrate': 1_000_000,
    'stopbits': serial.STOPBITS_ONE,
    'parity': serial.PARITY_NONE,
    'bytesize': serial.EIGHTBITS,
}


class BoomLogger:
    def __init__(self, filename='boom-log.csv'):
        print('connecting to Teensy... ', end='')
        devices = [dev for dev in comports() if dev.description == 'USB Serial']
        if len(devices) == 0:
            raise RuntimeError("Couldn't find Teensy")
        elif len(devices) == 1:
            dev = devices[0]
        else:
            raise RuntimeError(f'Found more than one Teensy: {devices}')
        self.serial = serial.Serial(dev.device, **SENSOR_PARAMS)
        print('Connected')

        self.thread = None
        self.isRunning = True
        self.receivingData = False
        self.data: List[BoomData] = []
        self.filename = filename
    
    @property
    def current(self):
        return self.data[-1]

    def __read(self) -> BoomData:
        self.serial.reset_input_buffer()
        self.serial.read_until(HEADER)
        data_bytes = self.serial.read(struct.calcsize(DATAFMT))
        yaw, pitch, x, y, dx, dy, ddx, ddy, ddz, temp = struct.unpack(DATAFMT, data_bytes)
        return BoomData(time.time(), yaw, pitch, x, y, dx, dy, ddx, ddy, ddz, temp)

 
    # Runs on a separate thread for reading serial data
    def __backgroundThread(self):
        time.sleep(0.1) # wait a bit before starting
        while (self.isRunning):
            packet = self.__read()
            self.data.append(packet)
            self.receivingData = True
    
    '''
    Starts logging the data from the boom encoders at approximately 1kHz.
    Logging is executed on a background thread, but this function will block the current thread until logging begins.
    The boom only start sending data once the vertical axis has been indexed.
    '''
    def __enter__(self):
        if self.thread == None:
            self.thread = Thread(target=self.__backgroundThread, daemon=True)
            self.thread.start()
            # Block till we start receiving values
            while not self.receivingData:
                time.sleep(0.1)
        else:
            raise RuntimeError('Thread already allocated')
        return self


    '''
    Stops logging data from the boom and saves the logged data to a CSV file.
    '''
    def __exit__(self, *_):
        self.isRunning = False
        self.thread.join()
        self.serial.close()
        print('Logging stopped...')
        # write to csv
        print('Writing data to CSV file...')
        with open(self.filename, 'w', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(['time[epoch]', 'yaw[counts]', 'pitch[counts]', 'x[m]', 'y[m]', 'dx[m/s]', 'dy[m/s]', 'ddx[g]', 'ddy[g]', 'ddz[g]', 'temperature[C]'])
            writer.writerows(self.data)



if __name__ == '__main__':
    # basic usage
    with BoomLogger() as logger:
        while True:
            try:
                print(f'{logger.current.ddx:.3f}')
                time.sleep(0.1)
            except KeyboardInterrupt:
                break
