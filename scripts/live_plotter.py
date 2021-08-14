import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from recieve_comms import BoomLogger
from collections import deque
import numpy as np


if __name__ == '__main__':
    logger = BoomLogger()
    logger.start()

    fig, ax = plt.subplots()
    xdata, ydata = deque(maxlen=5000), deque(maxlen=5000) # only store about 5s of data
    line, = plt.plot([], [])

    def init():
        ax.set_xlim(-2, 2)
        ax.set_ylim(0, 2)
        return line,

    # animation function. This is called sequentially
    def update(frame):
        data = logger.data[-1]
        xdata.append(data.x)
        ydata.append(data.y)
        line.set_data(xdata, ydata)
        return line,

    # plotting
    animation = FuncAnimation(fig, update, init_func=init, blit=True, interval=10)
    plt.show()

    time = np.array([packet.time for packet in logger.data])
    diff = np.diff(time)
    print(np.mean(diff))

    logger.stop()
    