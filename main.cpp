#include "mbed.h"
#include "MMC5603NJ.h"

int main(void) {

    I2C i2c = I2C(I2C_SDA, I2C_SCL);
    MMC5603NJ magSensor = MMC5603NJ(i2c, 140);
    int test;

    while (true) (
        test = magSensor.getProductID();
        printf("Product ID: %d\n", test);
        ThisThread::sleep_for(1000);
    )

    return 0;
}