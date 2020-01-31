#include "MMC5603NJ.h"

MMC5603NJ::MMC5603NJ(I2C ext_i2c, int odr) {
    i2c = ext_i2c
    _odr = odr;
    setODR(_odr);
}

MMC5603NJ::setODR(int odr) {
    if (odr >=0 && odr <= 255) {
        return(!i2c.write(ODR, (char*) &odr, 1))
    }
    else
    {
        return false;
    }
}

MMC5603NJ::startContinuousMode(bool set) {
    

}
