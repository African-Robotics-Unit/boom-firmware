import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# TODO
# - make the plot a 3x3 grid
# - estimate velocity with dx/dt to check PLL estimator

pll_bandwidth = 100 # rad/s

boom_data = pd.read_csv(f'pll_data/pll-10k-{pll_bandwidth}.csv')

boom_data['time[epoch]'] -= boom_data['time[epoch]'][0]

dt = boom_data['time[epoch]'].diff()
print(f'Average sample time of {1/dt.mean()} Hz')

# convert encoder counts to position
boom_data['yaw[counts]'] = boom_data['yaw[counts]'].apply(lambda count: (count / (4000*4)) * 2*np.pi * 2.558)
boom_data['pitch[counts]'] = boom_data['pitch[counts]'].apply(lambda count: (count / (4000*4)) * 2*np.pi * 2.475)


fig1, ax1 = plt.subplots()
ax1.set_title(f'Boom PLL position estimate\n{pll_bandwidth} rad/s bandwidth')
ax1.plot(boom_data['time[epoch]'], boom_data['pitch[counts]'], label='y actual')
ax1.plot(boom_data['time[epoch]'], boom_data['yaw[counts]'], label='x actual')
ax1.plot(boom_data['time[epoch]'], boom_data['y[m]'], label='y estimate')
ax1.plot(boom_data['time[epoch]'], boom_data['x[m]'], label='x estimate')
ax1.set(xlabel='time [s]', ylabel='position [m]')
ax1.legend()
plt.tight_layout()


fig2, ax2 = plt.subplots()
ax2.set_title(f'PLL position estimate error\n{pll_bandwidth} rad/s bandwidth')
ax2.plot(boom_data['time[epoch]'], abs(boom_data['pitch[counts]'] - boom_data['y[m]']), label='y error')
ax2.plot(boom_data['time[epoch]'], abs(boom_data['yaw[counts]'] - boom_data['x[m]'])**2, label='x error')
ax2.set(xlabel='time [s]', ylabel='|error| [m]')
ax2.legend()
plt.tight_layout()


fig5, ax5 = plt.subplots()
ax5.set_title(f'Hopper PLL velocity estimate\n{pll_bandwidth} rad/s bandwidth')
ax5.plot(boom_data['time[epoch]'], boom_data['dy[m/s]'], label='vertical')
ax5.plot(boom_data['time[epoch]'], boom_data['dx[m/s]'], label='horizontal')
ax5.set(xlabel='time [s]', ylabel='Speed [m/s]')
ax5.legend()
plt.tight_layout()


plt.show()
