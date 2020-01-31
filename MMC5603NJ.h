#include "mbed.h"

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

#define CTRL_2_CONTINUOUS_START 0x10


class MMC5603NJ { 

public:

    MMC5603NJ (I2C ext_i2c, PinName sda, PinName, scl);

    bool setODR(int odr);
    bool startContinuousMode();
    int getProductID();
    float getMag();

private:

    I2C i2c;
    int _odr;

}