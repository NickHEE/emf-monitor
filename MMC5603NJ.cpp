#include "MMC5603NJ.h"

MMC5603NJ::MMC5603NJ(I2C ext_i2c, int odr) {
    i2c = ext_i2c;
    _odr = odr;
    setODR(_odr);
}

MMC5603NJ::setODR(int odr) {
    
    if (odr >=0 && odr <= 255) {
        char[] data = {ODR, (char) odr};

        return(!i2c.write(MMC5603NJ_WRITE, data, 2))
    }
    return false;
}

MMC5603NJ::startContinuousMode() {
    
    const char[] data1 = {CTRL_0, CTRL_0_CONTINUOUS};
    const char[] data2 = {CTRL_2, CTRL_2_CONTINUOUS_START};
    
    if (!i2c.write(MMC5603NJ_WRITE, data1, 2)) {
        
        if(!i2c.write(MMC5603NJ_WRITE, data2, 2 )) {

            return true;
        }
    }
    return false;
}

MMC5603NJ::getProductID() {

    const char data* = PRODUCT_ID;
    char dataOut = 0x00;

    if (!i2c.write(MMC5603NJ_WRITE, data, 1)) {
        return (int) i2c.read(MMC5603NJ_READ, &dataOut, 1);
    };
}


