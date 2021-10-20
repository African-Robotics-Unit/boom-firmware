import numpy as np
import pandas as pd
from scipy.linalg import inv
from scipy.stats import describe
import matplotlib.pyplot as plt


data = np.loadtxt('boom-log.csv', delimiter=',', skiprows=1)
data[:,0] -= data[0,0]
dt = np.mean(np.diff(data[:,0]))
num_rows, num_cols = data.shape

print(describe(data[0:100,6]))


x = np.array([[0.3, 0, 0]]).T # state
P = np.diag([1, 1, 1]) # state covariance
F = np.array([[1, dt, 0.5*dt*dt], [0,  1, dt], [0, 0, 1]]) # process model
Γ = np.array([[0.5*dt*dt, dt, 1]]).T
j = 1 # max jerk [m/s^3]
Q = Γ * (j*dt)**2 * Γ.T # process noise covariance
H = np.array([[1, 0, 0], [0, 0, 1]]) # measurement function
R = np.diag([1e-12, 2e-3]) # measurement noise covariance

zs = data[:,[2,8]]
xs, cov = [], []
for z in zs:
    z = np.array([[z[0], z[1]]]).T
    # predict
    x = F @ x
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

xs = np.reshape(xs, (-1,3))

fig, ax = plt.subplots(3, sharex=True)
fig.suptitle(f'Boom data [500Hz]')

ax[0].set_title('Position')
ax[0].step(data[:,0], data[:,4], label='y', where='post')
ax[0].plot(data[:,0], xs[:,0], label='y KF')
ax[0].set(ylabel='m')
ax[0].legend()

ax[1].set_title('Velocity')
ax[1].plot(data[:,0], data[:,6], label='y PLL')
ax[1].plot(data[:,0], xs[:,1], label='y KF')
ax[1].set(ylabel='m/s')
ax[1].legend()

ax[2].set_title(f'Acceleration')
ax[2].plot(data[:,0], data[:,8], label='y')
ax[2].plot(data[:,0], xs[:,2], label='y KF')
ax[2].set(xlabel='time [s]', ylabel="m/s²")
ax[2].legend()

plt.tight_layout()
plt.show()