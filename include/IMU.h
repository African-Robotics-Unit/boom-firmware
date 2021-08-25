
#include <SparkFunLSM9DS1.h>

class IMU: public LSM9DS1 {
    public :
    // custom calibrate function to override one in LSM9DS1 library
    void customCalibrate(bool autoCalc) {
        uint8_t samples = 0;
        int ii;
        int32_t aBiasRawTemp[3] = {0, 0, 0};
        int32_t gBiasRawTemp[3] = {0, 0, 0};
        
        // Turn on FIFO and set threshold to 32 samples
        enableFIFO(true);
        setFIFO(FIFO_THS, 0x1F);
        while (samples < 0x1F) {
            samples = (xgReadByte(FIFO_SRC) & 0x3F); // Read number of stored samples
        }
        for(ii = 0; ii < samples ; ii++) {	// Read the gyro data stored in the FIFO
            readGyro();
            gBiasRawTemp[0] += gx;
            gBiasRawTemp[1] += gy;
            gBiasRawTemp[2] += gz;
            readAccel();
            aBiasRawTemp[0] += ax;
            aBiasRawTemp[1] += ay;
            aBiasRawTemp[2] += az;
        }  
        for (ii = 0; ii < 3; ii++) {
            gBiasRaw[ii] = gBiasRawTemp[ii] / samples;
            gBias[ii] = calcGyro(gBiasRaw[ii]);
            aBiasRaw[ii] = aBiasRawTemp[ii] / samples;
            aBias[ii] = calcAccel(aBiasRaw[ii]);
        }
        enableFIFO(false);
        setFIFO(FIFO_OFF, 0x00);
        
        if (autoCalc) _autoCalc = true;
    }
};
