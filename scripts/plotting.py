import pandas as pd
import numpy as np
import matplotlib.pyplot as plt


def plot(filename):

    data = pd.read_csv(filename)

    data['time'] -= data['time'][0]

    data['dt'] = data['time'].diff() * 1e6
    avg_freq = 1e6 / data['dt'].mean()

    num = len(data['time'])
    dt = 0.001
    data['time'] = np.arange(start=0, stop=dt*num, step=dt)

    fig, ax = plt.subplots(3, sharex=True)
    fig.suptitle(f'Boom data [{avg_freq:.0f}Hz]')

    ax[0].set_title('Position')
    ax[0].step(data['time'], data['x_enc'], label='x enc', where='post')
    ax[0].step(data['time'], data['y_enc'], label='y enc', where='post')
    # ax[0].plot(data['time'], data['x_pll'], label='x pll')
    # ax[0].plot(data['time'], data['y_pll'], label='y pll')
    ax[0].plot(data['time'], data['x_kf'], label='x kf')
    ax[0].plot(data['time'], data['y_kf'], label='y kf')
    ax[0].set(ylabel='m')
    ax[0].legend()

    ax[1].set_title('Velocity')
    # ax[1].plot(data['time'], data['dx_pll'], label='x pll')
    # ax[1].plot(data['time'], data['dy_pll'], label='y pll')
    ax[1].plot(data['time'], data['dx_kf'], label='x kf')
    ax[1].plot(data['time'], data['dy_kf'], label='y kf')
    ax[1].set(ylabel='m/s')
    ax[1].legend()

    ax[2].set_title(f'Acceleration')
    # ax[2].plot(data['time'], data['ddx_imu'], label='x imu')
    ax[2].plot(data['time'], data['ddx_kf'], label='x kf')
    # ax[2].plot(data['time'], data['ddy_imu'], label='y imu')
    ax[2].plot(data['time'], data['ddy_kf'], label='y kf')
    # ax[2].plot(data['time'], data['ddz_imu'], label='z IMU')
    ax[2].set(xlabel='time [s]', ylabel="m/s^2")
    ax[2].legend()

    plt.tight_layout()
    plt.show()


if __name__ == '__main__':
    plot(filename='boom-log.csv')