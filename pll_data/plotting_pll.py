import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# TODO
# - estimate velocity with dx/dt to check PLL estimator

pll_bandwidth = 1000 # rad/s

boom_data = pd.read_csv(f'pll_data/100k-{pll_bandwidth}.csv')

boom_data['time[epoch]'] -= boom_data['time[epoch]'][0]

dt = boom_data['time[epoch]'].diff()
print(f'Average sample time of {1/dt.mean()} Hz')

# convert encoder counts to position
boom_data['yaw[counts]'] = boom_data['yaw[counts]'].apply(lambda count: (count / (4096*4)) * 2*np.pi * 2.558)
boom_data['pitch[counts]'] = boom_data['pitch[counts]'].apply(lambda count: (count / (4096*4)) * 2*np.pi * 2.475)

fig, ax = plt.subplots(3)
fig.suptitle(f'PLL {pll_bandwidth} rad/s bandwidth')


ax[0].set_title(f'Position')
ax[0].step(boom_data['time[epoch]'], boom_data['pitch[counts]'], label='y actual', where='post')
ax[0].step(boom_data['time[epoch]'], boom_data['yaw[counts]'], label='x actual', where='post')
ax[0].plot(boom_data['time[epoch]'], boom_data['y[m]'], label='y estimate')
ax[0].plot(boom_data['time[epoch]'], boom_data['x[m]'], label='x estimate')
ax[0].set(xlabel='time [s]', ylabel='position [m]')
ax[0].legend()


ax[1].set_title(f'Position error')
ax[1].plot(boom_data['time[epoch]'], abs(boom_data['pitch[counts]'] - boom_data['y[m]']), label='y error')
ax[1].plot(boom_data['time[epoch]'], abs(boom_data['yaw[counts]'] - boom_data['x[m]']), label='x error')
ax[1].set(xlabel='time [s]', ylabel='|error| [m]')
ax[1].legend()


ax[2].set_title(f'Velocity estimate')
ax[2].plot(boom_data['time[epoch]'], boom_data['dy[m/s]'], label='vertical')
ax[2].plot(boom_data['time[epoch]'], boom_data['dx[m/s]'], label='horizontal')
ax[2].set(xlabel='time [s]', ylabel='Speed [m/s]')
ax[2].legend()

plt.tight_layout()
plt.show()
