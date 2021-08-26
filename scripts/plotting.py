import pandas as pd
import numpy as np
import matplotlib.pyplot as plt


def plot(filename):

    data = pd.read_csv(filename)

    data['time[epoch]'] -= data['time[epoch]'][0]

    data['dt[us]'] = data['time[epoch]'].diff() * 1e6
    avg_freq = 1e6 / data['dt[us]'].mean()

    fig, ax = plt.subplots(3, sharex=True)
    fig.suptitle(f'Boom data [{avg_freq:.0f}Hz]')

    ax[0].set_title('Position')
    ax[0].plot(data['time[epoch]'], data['x[m]'], label='x')
    ax[0].plot(data['time[epoch]'], data['y[m]'], label='y')
    ax[0].set(ylabel='m')
    ax[0].legend()

    ax[1].set_title('Velocity')
    ax[1].plot(data['time[epoch]'], data['dx[m/s]'], label='x')
    ax[1].plot(data['time[epoch]'], data['dy[m/s]'], label='y')
    ax[1].set(ylabel='m/s')
    ax[1].legend()

    ax[2].set_title(f'Acceleration')
    ax[2].plot(data['time[epoch]'], data['ddz[g]'], label='x')
    ax[2].plot(data['time[epoch]'], data['ddx[g]'], label='y')
    # ax[2].plot(data['time[epoch]'], data['ddy[g]'], label='z')
    ax[2].set(xlabel='time [s]', ylabel="g's")
    ax[2].legend()

    plt.tight_layout()
    plt.show()


if __name__ == '__main__':
    plot(filename='boom-log.csv')