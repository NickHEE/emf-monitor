#include "mbed.h"

#define Xout0 	0x00 
#define Xout1 	0x01
#define Yout0 	0x02 
#define Yout1 	0x03 
#define Zout0 	0x04 
#define Zout1 	0x05 
#define Xout2 	0x06 
#define Yout2 	0x07
#define Zout2 	0x08 
#define Tout 	0x09 
#define Status1 	0x18
#define ODR 	0x1A
#define Internal_control_0 	0x1B 
#define Internal_control_1 	0x1C 
#define Internal_control_2 	0x1D
#define ST_X_TH 	0x1E 
#define ST_Y_TH 	0x1F
#define ST_Z_TH 	0x20 
#define ST_X 	0x27
#define ST_Y 	0x28 
#define ST_Z 	0x29 
#define Product ID 	0x39 

class MMC5603NJ { 

public:

    MMC5603NJ (I2C ext_i2c, PinName sda, PinName, scl);

    void init();
    bool setODR(int odr);
    bool startContinuousMode();
    float getMag();

private:

    I2C i2c;
    int _odr;

}