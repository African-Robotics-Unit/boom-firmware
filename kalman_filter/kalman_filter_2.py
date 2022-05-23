import numpy as np
import pandas as pd
from scipy.linalg import inv
from scipy.stats import describe
from scipy.signal import savgol_filter
import matplotlib.pyplot as plt

#! Uses accelerometer as input

data = pd.read_csv('boom-log.csv')
dt = 0.001
num_rows, num_cols = data.shape

data['time'] = np.arange(start=0, stop=dt*len(data['time']), step=dt)

print(describe(data['ddx_imu'].iloc[0:100]))


x = np.array([[0, 0]]).T # state
P = np.diag([1, 1]) # state covariance
F = np.array([  [1, dt],
                [0, 1]]) # process model
B = np.array([  [0.5*dt*dt],
                [dt]]) # control function
Γ = np.array([[dt, 1]]).T
Q = Γ * 2e-4 * Γ.T # process noise covariance (accelerometer noise)
H = np.array([[1, 0]]) # measurement function
R = np.diag([1e-4]) # measurement noise covariance (encoder noise)

# encoder theoretical variance = 1/12*(b-a)^2
# flex in the boom results in this being slightly higher

xs, cov = [], []
for index, zz in data[['y_enc', 'ddy_imu']].iterrows():
    z = np.array([[zz['y_enc']]]).T
    u = np.array([[zz['ddy_imu']]]).T
    # predict
    x = F @ x + B @ u
    P = F @ P @ F.T + Q
    #update
    S = H @ P @ H.T + R
    K = P @ H.T @ inv(S)
    y = z - H @ x
    x += K @ y
    P = P - K @ H @ P
    
    xs.append(x)
    cov.append(P)


xs, cov = np.array(xs), np.array(cov)

xs = np.reshape(xs, (-1,2))

fig, ax = plt.subplots(4, sharex=True)
fig.suptitle(f'Boom data [1000Hz]')
fig.set_size_inches(13.333, 7)

ax[0].set_title('Position')
ax[0].step(data['time'], data['y_enc'], label='Encoder', where='post', c='tab:blue')
ax[0].plot(data['time'], xs[:,0], label='Kalman filter', c='tab:orange')
ax[0].set(ylabel='m')
ax[0].legend()

ax[1].set_title('Velocity')
# ax[1].plot(data['time'], data['dy_pll'], label='PLL estimator')
sg_est = savgol_filter(data['y_enc'], window_length=51, polyorder=3, deriv=1, delta=dt)
ax[1].plot(data['time'], sg_est, label='Savitzky-Golay filter', c='tab:green')
ax[1].plot(data['time'], xs[:,1], label='Kalman filter', c='tab:orange')
ax[1].set(ylabel='m/s')
ax[1].legend()

# zoomed in data
# ax[2].set_title('Position')
# ax[2].step(data['time'], data['y_enc'], label='Encoder', where='post')
# ax[2].plot(data['time'], xs[:,0], label='Kalman Filter')
# ax[2].set(ylabel='m')
# ax[2].legend()

# ax[3].set_title('Velocity')
# ax[3].plot(data['time'], data['dy_pll'], label='PLL estimator')
# ax[3].plot(data['time'], xs[:,1], label='Kalman Filter')
# ax[3].set(ylabel='m/s')
# ax[3].legend()

# xlim = [8.350, 8.365]
# ax[2].set_xlim(xlim)
# ax[2].set_ylim([0.62830, 0.61524])
# ax[3].set_xlim(xlim)
# ax[3].set_ylim([-0.6, -0.8])

ax[2].set_title(f'Acceleration')
ax[2].plot(data['time'], data['ddy_imu'], label='IMU', c='tab:blue')
sg_est = savgol_filter(data['y_enc'], window_length=51, polyorder=3, deriv=2, delta=dt)
ax[2].plot(data['time'], sg_est, label='Savitzky-Golay filter', c='tab:green')
ax[2].set(xlabel='time', ylabel="m/s²")
ax[2].legend()

# plot position error
ax[3].set_title('Position Error')
ax[3].plot(data['time'], np.abs(data['y_enc']-xs[:,0]), label='error')
ax[3].set(ylabel='m')
ax[3].legend()


plt.tight_layout()
plt.show()