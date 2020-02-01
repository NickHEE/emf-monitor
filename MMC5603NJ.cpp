#include "MMC5603NJ.h"

MMC5603NJ::MMC5603NJ(I2C* ext_i2c, int odr, int BW) {
    i2c = ext_i2c;
    _odr = odr;
    setODR(_odr);
    setBW(BW)
}

bool MMC5603NJ::setODR(int odr) {
    char data[2];
    data[0] = ODR;
    data[1] = (char) odr;
    
    if (odr >=0 && odr <= 255) {
        return(!i2c->write(MMC5603NJ_WRITE, data, 2));
    }
    return false;
}

bool MMC5603NJ::setBW(int BW) {
    char data[2];
    data[0] = CTRL_1;
    data[1] = (char) BW;

    if (BW == CTRL_1_BW_75HZ || BW == CTRL_1_BW_150HZ || BW == CTRL_1_BW_255HZ || BW == CTRL_1_BW_1000HZ) {
        return(!i2c->write(CTRL_1, data, 2));
    }
    return false;
}

bool MMC5603NJ::startContinuousMode() {
    char data1[2];
    data1[0] = CTRL_0;
    data1[1] = CTRL_0_CONTINUOUS;

    char data2[2];
    data2[0] = CTRL_2;
    data2[1] = CTRL_2_CONTINUOUS_START;
    
    if (!i2c->write(MMC5603NJ_WRITE, data1, 2)) {
        
        if(!i2c->write(MMC5603NJ_WRITE, data2, 2 )) {
            return true;
        }
    }
    return false;
}

int MMC5603NJ::getProductID() {

    const char data = PRODUCT_ID;
    char dataOut = 0x01;

    if (!i2c->write(MMC5603NJ_WRITE, &data, 1)) {
        i2c->read(MMC5603NJ_READ, &dataOut, 1);
        return (int) dataOut;
    };
    return -1;
}

float MMC5603NJ::takeMeasurement() {

}


