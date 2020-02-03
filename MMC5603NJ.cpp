#include "MMC5603NJ.h"


MMC5603NJ::MMC5603NJ(I2C* ext_i2c, int odr, int BW) {
    i2c = ext_i2c;
    _odr = odr;
    setODR(_odr);
    setBW(BW);
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
    char dataOut = 0x00;

    if (!i2c->write(MMC5603NJ_WRITE, &data, 1)) {
        i2c->read(MMC5603NJ_READ, &dataOut, 1);
        return (int) dataOut;
    };
    return -1;
}

float32_t MMC5603NJ::getMeasurement() {

    char status_reg = STATUS_1;
    char status;
    float32_t x, y, z;

    char axisData[9];
    char xout0 = X_OUT_0;
    
    do {
        if (!i2c->write(MMC5603NJ_WRITE, &status_reg, 1)) {
            i2c->read(MMC5603NJ_READ, &status, 1);
            //printf("STATUS: %d", status);
        }
    }
    while(!(status & 0x40));

    if (!i2c->write(MMC5603NJ_WRITE, &xout0, 1)) {
        i2c->read(MMC5603NJ_READ, axisData, 9);
    }

    // printf("Xout0: %d, Xout1: %d, Xout2: %d\n"
    //        "Yout0: %d, Yout1: %d, Yout2: %d\n"
    //        "Zout0: %d, Zout1: %d, Zout2: %d\n", 
    // axisData[0],axisData[1],axisData[6],axisData[2],axisData[3],axisData[7],axisData[4],axisData[5],axisData[8]);

    x = (float32_t) (((axisData[0] << 12) + (axisData[1] << 4) + (axisData[6] >> 4)) * sensitivity) - 32768;
    y = (float32_t) (((axisData[2] << 12) + (axisData[3] << 4) + (axisData[7] >> 4)) * sensitivity) - 32768;
    z = (float32_t) (((axisData[4] << 12) + (axisData[5] << 4) + (axisData[8] >> 4)) * sensitivity) - 32768;

    return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));

    //return sqrt(pow((float32_t)axisData[0], 2) + pow((float32_t)axisData[1], 2) + pow((float32_t)axisData[2], 2));
}


