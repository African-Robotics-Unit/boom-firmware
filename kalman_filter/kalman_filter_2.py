import numpy as np
import pandas as pd
from scipy.linalg import inv
from scipy.stats import describe
import matplotlib.pyplot as plt

#! Uses accelerometer as input

data = pd.read_csv('boom-log.csv')
dt = 0.001
num_rows, num_cols = data.shape

data['time'] = np.arange(start=0, stop=dt*len(data['time']), step=dt)

print(describe(data['ddx_imu'].iloc[0:100]))


x = np.array([[0.3, 0]]).T # state
P = np.diag([1, 1]) # state covariance
F = np.array([  [1, dt],
                [0, 1]]) # process model
B = np.array([  [0, 0.5*dt*dt],
                [0, dt]]) # control function
Γ = np.array([[dt, 1]]).T
Q = Γ * 1e-8 * Γ.T # process noise covariance
H = np.array([[1, 0]]) # measurement function
R = np.diag([1e-8]) # measurement noise covariance

xs, cov = [], []
for index, zz in data[['x_enc', 'ddx_imu']].iterrows():
    z = np.array([[zz['x_enc']]]).T
    u = np.array([[0, zz['ddx_imu']]]).T
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

fig, ax = plt.subplots(2, sharex=True)
fig.suptitle(f'Boom data [1000Hz]')

ax[0].set_title('Position')
ax[0].step(data['time'], data['x_enc'], label='y', where='post')
ax[0].plot(data['time'], xs[:,0], label='y KF')
ax[0].set(ylabel='m')
ax[0].legend()

ax[1].set_title('Velocity')
ax[1].plot(data['time'], data['dx_pll'], label='y PLL')
ax[1].plot(data['time'], xs[:,1], label='y KF')
ax[1].set(ylabel='m/s')
ax[1].legend()

# ax[2].set_title(f'Acceleration')
# ax[2].plot(data['time'], data['ddy_imu'], label='y')
# ax[2].plot(data['time'], xs[:,2], label='y KF')
# ax[2].set(xlabel='time [s]', ylabel="m/s²")
# ax[2].legend()

plt.tight_layout()
plt.show()