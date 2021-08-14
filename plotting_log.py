import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# TODO

data = pd.read_csv(f'boom-log-data.csv')

data['time[epoch]'] -= data['time[epoch]'][0]

data['dt[us]'] = data['time[epoch]'].diff() * 1e6
avg_freq = 1e6 / data['dt[us]'].mean()

print(f'Average frequency: {avg_freq} Hz')

# convert encoder counts to position
data['yaw[counts]'] = data['yaw[counts]'].apply(lambda count: (count / (4096*4)) * 2*np.pi * 2.558)
data['pitch[counts]'] = data['pitch[counts]'].apply(lambda count: (count / (4096*4)) * 2*np.pi * 2.475)

# boom position data
fig1, ax1 = plt.subplots()
ax1.set_title('Boom position')
ax1.plot(data['time[epoch]'], data['pitch[counts]'], label='y actual')
ax1.plot(data['time[epoch]'], data['yaw[counts]'], label='x actual')
ax1.plot(data['time[epoch]'], data['y[m]'], label='y estimate')
ax1.plot(data['time[epoch]'], data['x[m]'], label='x estimate')
ax1.set(xlabel='time [s]', ylabel='position [m]')
ax1.legend()
plt.tight_layout()


# boom velocity data
fig2, ax2 = plt.subplots()
ax2.set_title('Boom velocity')
ax2.plot(data['time[epoch]'], abs(data['pitch[counts]'] - data['y[m]']), label='y error')
ax2.plot(data['time[epoch]'], abs(data['yaw[counts]'] - data['x[m]'])**2, label='x error')
ax2.set(xlabel='time [s]', ylabel='|error| [m]')
ax2.legend()
plt.tight_layout()


fig5, ax5 = plt.subplots()
ax5.set_title(f'Boom acceleration')
ax5.plot(data['time[epoch]'], data['ddx[m/s^2]'], label='x')
ax5.plot(data['time[epoch]'], data['ddy[m/s^2]'], label='y')
ax5.plot(data['time[epoch]'], data['ddz[m/s^2]'], label='z')
ax5.set(xlabel='time [s]', ylabel='Acceleration [m/s^2]')
ax5.legend()
plt.tight_layout()


# boom velocity data
fig3, ax3 = plt.subplots()
ax3.set_title('Serial recieve dt')
ax3.plot(data['time[epoch]'], data['dt[us]'])
ax3.axhline(y=data['dt[us]'].mean(), c='r')
ax3.set(xlabel='time [s]', ylabel='dt [us]')
plt.tight_layout()


plt.show()
