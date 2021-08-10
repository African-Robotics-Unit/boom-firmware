import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from scripts.recieve_comms import BoomLogger
import numpy as np


if __name__ == '__main__':
    logger = BoomLogger()
    logger.startReadThread()

    fig, ax = plt.subplots()
    xdata, ydata = [], []
    line, = plt.plot([], [])

    def init():
        ax.set_xlim(-250, 250)
        ax.set_ylim(-250, 250)
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

    logger.disconnect()
    