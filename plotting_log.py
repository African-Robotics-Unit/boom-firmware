import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# TODO

data = pd.read_csv(f'pll_data/10k-100.csv')

data['time[epoch]'] -= data['time[epoch]'][0]

data['dt[us]'] = data['time[epoch]'].diff() * 1e6
avg_freq = 1e6 / data['dt[us]'].mean()

print(f'Average frequency: {avg_freq} Hz')

fig, ax = plt.subplots(3)

fig.suptitle(f'Boom data at {avg_freq:.1f}kHz')


ax[0].set_title('Boom position')
ax[0].plot(data['time[epoch]'], data['x[m]'], label='x')
ax[0].plot(data['time[epoch]'], data['y[m]'], label='y')
ax[0].set(xlabel='time [s]', ylabel='position [m]')
ax[0].legend()


ax[1].set_title('Boom velocity')
ax[1].plot(data['time[epoch]'], data['dx[m/s]'], label='x')
ax[1].plot(data['time[epoch]'], data['dy[m/s]'], label='y')
ax[1].set(xlabel='time [s]', ylabel='velocity [m/s]')
ax[1].legend()


ax[2].set_title(f'Boom acceleration')
ax[2].plot(data['time[epoch]'], data['ddz[g]'], label='x')
ax[2].plot(data['time[epoch]'], data['ddx[g]'], label='y')
# ax[2].plot(data['time[epoch]'], data['ddy[g]'], label='z')
ax[2].set(xlabel='time [s]', ylabel="Acceleration [g's]")
ax[2].legend()


# # boom velocity data
# ax[3].set_title('Serial recieve dt')
# ax[3].plot(data['time[epoch]'], data['dt[us]'])
# ax[3].axhline(y=data['dt[us]'].mean(), c='r')
# ax[3].set(xlabel='time [s]', ylabel='dt [us]')


plt.tight_layout()
plt.show()
