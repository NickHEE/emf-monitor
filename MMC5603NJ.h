#include "mbed.h"
#include "arm_math.h"

// Register Addresses
#define X_OUT_0 	0x00 
#define X_OUT_1 	0x01
#define Y_OUT_0 	0x02 
#define Y_OUT_1 	0x03 
#define Z_OUT_0 	0x04 
#define Z_OUT_1 	0x05 
#define X_OUT_2 	0x06 
#define Y_OUT_2 	0x07
#define Z_OUT_2 	0x08 
#define T_OUT 	    0x09 
#define STATUS_1 	0x18
#define ODR 	    0x1A
#define CTRL_0 	    0x1B 
#define CTRL_1 	    0x1C 
#define CTRL_2 	    0x1D
#define ST_X_TH 	0x1E 
#define ST_Y_TH 	0x1F
#define ST_Z_TH 	0x20 
#define ST_X 	    0x27
#define ST_Y 	    0x28 
#define ST_Z 	    0x29 
#define PRODUCT_ID 	0x39

// Device Addresses 
#define MMC5603NJ_WRITE 0x60
#define MMC5603NJ_READ 0x61

// Register Configurations
#define CTRL_0_CONTINUOUS_AUTO_SR 0xA0
#define CTRL_0_CONTINUOUS 0x80
#define CTRL_0_TAKE_MEASUREMENT 0x01

#define CTRL_1_BW_75HZ 0x00
#define CTRL_1_BW_150HZ 0x01
#define CTRL_1_BW_255HZ 0x02
#define CTRL_1_BW_1000HZ 0x03

#define CTRL_2_CONTINUOUS_START 0x10
#define CTRL_2_CONTINUOUS_START_HIPWR 0x90
#define CTRL_2_HIPWR 0x80

#define STATUS_1_MEAS_DONE 0x40

// Misc
#define T_WAIT_BW_75 6.6
#define T_WAIT_BW_150 3.5
#define T_WAIT_BW_255 2.0
#define T_WAIT_BW_HPOWER 1.2



class MMC5603NJ { 

public:

    MMC5603NJ (I2C* ext_i2c, int odr, int BW);

    bool setODR(int odr);
    bool setBW(int BW);
    bool startContinuousMode();
    bool takeMeasurement();
    bool setHiPWR();
    int getProductID();
    float32_t getMeasurement(bool wait);

    I2C* i2c;

private:

    int _odr;
    float32_t sensitivity = 0.0625;
};