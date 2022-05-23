
#include <SparkFunLSM9DS1.h>
#include <Wire.h>

// A custom subclass of SparkFun's LSM9DS1

class IMU: public LSM9DS1 {
    public :

    float ddx, ddy, ddz; // Acceleration readings in boom coordinate frame [m/s^2]
    float temperatureCelcius;

    // Accelerometer must be stationary when this is run
    void customCalibrate(int samples) {
        for (int i=0 ; i<samples ; i++) {
            while (!accelAvailable()); // wait for acceleration values
            readAccel();
            zeroOffset[X_AXIS] += ax;
            zeroOffset[Y_AXIS] += ay;
            zeroOffset[Z_AXIS] += az;
        }
        for (int i = 0; i < 3; i++) {
            zeroOffset[i] /= samples;
            aBiasRaw[i] = zeroOffset[i];
        }
        // determine angle of boom end
        phiOffset = atan2(zeroOffset[Y_AXIS], zeroOffset[X_AXIS]);
        _autoCalc = true;
    }

    void readAcceleration() {
        if (accelAvailable()) { readAccel(); }
        // scale and subtract offset
        float ddx_s = calcAccel(ax) * 9.81;
        float ddy_s = calcAccel(ay) * 9.81;
        float ddz_s = calcAccel(az) * 9.81;
        // rotate to world frame
        ddy = ddx_s*cos(-phiOffset) - ddy_s*sin(-phiOffset);
        ddz = ddy_s*cos(-phiOffset) + ddx_s*sin(-phiOffset);
        ddx = -ddz_s;
    }

    void readTemperature() {
        if (tempAvailable()) { readTemp(); }
        // 0b0 @ 25°C
        temperatureCelcius = (((float_t)temperature / 16.0f) + 25.0f);
    }


    // configure settings for the IMU
    // https://github.com/sparkfun/SparkFun_LSM9DS1_Arduino_Library/blob/master/examples/LSM9DS1_Settings/LSM9DS1_Settings.ino

    void setupGyro() {
        settings.gyro.enabled = false;  // Enable the gyro
        settings.gyro.scale = 245; // Set scale to +/-245dps
        settings.gyro.sampleRate = 6; // seems like the only way to disable the gyro
        settings.gyro.bandwidth = 0;
        settings.gyro.lowPowerEnable = true; // LP mode off
        settings.gyro.HPFEnable = true; // HPF disabled
        settings.gyro.HPFCutoff = 6; // HPF cutoff = 4Hz
        settings.gyro.flipX = false; // Don't flip X
        settings.gyro.flipY = false; // Don't flip Y
        settings.gyro.flipZ = false; // Don't flip Z
    }

    void setupAccel() {
        settings.accel.enabled = true; // Enable accelerometer
        settings.accel.enableX = true; // Enable X
        settings.accel.enableY = true; // Enable Y
        settings.accel.enableZ = true; // Enable Z
        settings.accel.scale = 8; // Set accel scale to +/-4g.
        settings.accel.sampleRate = 6; // Set accel to 10Hz.
        settings.accel.bandwidth = 2; // BW = 105Hz
        settings.accel.highResEnable = false; // Disable HR
        settings.accel.highResBandwidth = 0;
    }

    void setupMag() {
        settings.mag.enabled = false; // Enable magnetometer
        settings.mag.scale = 12; // Set mag scale to +/-12 Gs
        settings.mag.sampleRate = 0; // Set OD rate to 20Hz
        settings.mag.tempCompensationEnable = false;
        settings.mag.XYPerformance = 0; // low power mode
        settings.mag.ZPerformance = 0; // low power mode
        settings.mag.lowPowerEnable = true;
        settings.mag.operatingMode = 2; // power down
    }

    void setupTemperature() {
        settings.temp.enabled = true;
    }

    void updateSettings() {
        setupGyro();
        setupAccel();
        setupMag();
        constrainScales();
        calcgRes();
        calcaRes();
        calcmRes();
        initGyro(); // write settings to registers
        initAccel(); // write settings to registers
        initMag(); // write settings to registers
    }

    // runs through startup procedure
    // configures IMU settings and calibrates the accelerometer
    void initialise() {
        Wire.begin();
        Wire.setClock(400000); // 400kHz I2C
        if (!begin()) {
            Serial.println("Failed to initialize IMU");
            while (1);
        }
        updateSettings();
        sleepGyro(true);
        
        delay(100);
        customCalibrate(1000);
        delay(100);
    }

    private:
        float zeroOffset[3] = {0, 0, 0};
        float phiOffset = 0;
};
