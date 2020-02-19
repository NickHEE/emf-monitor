#include "mbed.h"
#include "test_env.h"
#include "battery_gauge_bq27441.h"


// Entry point into the tests
int main() {    
    //bool success = false;
    
    BatteryGaugeBq27441 monitor;
    I2C i2c(I2C_SDA, I2C_SCL);
    //int path = I2C i2c(I2C_SDA, I2C_SCL);
    monitor.init(&i2c, 0x55, 0x8000);
    monitor.enableBatteryDetect();
    
    int32_t data;
    int32_t temp;
    
    while(1)
    {
        monitor.getRemainingPercentage(&data);
        monitor.getTemperature(&temp);
        printf("Percentage: %d ", data);
        printf("Temperature: %d\n", temp);    
    }
/*    
    if (gpI2C != NULL) {        
        success = !Harness::run(specification);
        while(1)
        {
            if(isBatteryDetected()
                printf("high");    
        }
    } else {
        printf ("Unable to instantiate I2C interface.\n");
    }
*/    
}

// End Of File
