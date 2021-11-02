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


class Frame(NamedTuple):
    time: float
    x_enc: float
    x_pll: float
    x_kf: float
    y_enc: float
    y_pll: float
    y_kf: float
    dx_pll: float
    dx_kf: float
    dy_pll: float
    dy_kf: float
    ddx_imu: float
    ddx_kf: float
    ddy_imu: float
    ddy_kf: float
    ddz_imu: float
    temp: int


HEADER = bytes([0xAA, 0x55])
DATAFMT = '<15fi'


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
        self.data: List[Frame] = []
        self.filename = filename
    
    @property
    def current(self):
        return self.data[-1]

    def __read(self) -> Frame:
        self.serial.reset_input_buffer()
        self.serial.read_until(HEADER)
        data_bytes = self.serial.read(struct.calcsize(DATAFMT))
        data = struct.unpack(DATAFMT, data_bytes)
        return Frame(time.time(), *data)

 
    # Runs on a separate thread for reading serial data
    def __backgroundThread(self):
        time.sleep(0.1) # wait a bit before starting
        while (self.isRunning):
            frame = self.__read()
            self.data.append(frame)
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
            writer.writerow(self.data[0]._fields)
            writer.writerows(self.data)



if __name__ == '__main__':
    # basic usage
    with BoomLogger() as logger:
        while True:
            try:
                print(f'{logger.current.y_enc:.4f}')
                time.sleep(0.1)
            except KeyboardInterrupt:
                break
