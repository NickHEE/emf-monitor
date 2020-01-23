// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

//#define USE_MQTT
//py -2 -m mbed compile -m NUCLEO_L496ZG -t GCC_ARM --profile toolchain_debug.json

#include <stdlib.h>
#include <math.h>
#include <vector>
#include "mbed.h"
#include "arm_math.h"
#ifdef USE_MQTT
#include "iothubtransportmqtt.h"
#else
#include "iothubtransporthttp.h"
#endif
#include "iothub_client_core_common.h"
#include "iothub_client_ll.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/agenttime.h"
#include "jsondecoder.h"
#include "button.hpp"
#include "filter.h"
//#include "_XNucleoIKS01A2.h"

#define APP_VERSION "1.2"
#define IOT_AGENT_OK CODEFIRST_OK

#define GET_MAG_READING_START 1
#define GET_MAG_TAKE_MEASUREMENT 2
#define AZURE_START 3



float32_t magIn;
float32_t magOut;

std::vector<float> magReadings;

Ticker mag_ISR_timer;
Ticker mag_reading_timer;


#include "azure_certs.h"

/* The following is the message we will be sending to Azure */
typedef struct IoTDevice_t {
    char* ObjectName;
    char* ObjectType;
    char* Version;
    const char* ReportingDevice;
    int   MagneticField;
    int   ElectricField;
    float Temperature;
    int   Humidity;
    int   Pressure;
    int   Tilt;
    int   ButtonPress;
    char* TOD;
    } IoTDevice;

#define IOTDEVICE_MSG_FORMAT       \
   "{"                             \
     "\"ObjectName\":\"%s\","      \
     "\"ObjectType\":\"%s\","      \
     "\"Version\":\"%s\","         \
     "\"ReportingDevice\":\"%s\"," \
     "\"MagneticField\":\"%d\","   \
     "\"ElectricField\":\"%d\","   \
     "\"Temperature\":\"%.02f\","  \
     "\"Humidity\":\"%d\","        \
     "\"Pressure\":\"%d\","        \
     "\"Tilt\":\"%d\","            \
     "\"ButtonPress\":\"%d\","     \
     "\"TOD\":\"%s UTC\""          \
   "}"

/* initialize the expansion board && sensors */

#define ENV_SENSOR "IKS01A2"
#include "XNucleoIKS01A2.h"
static HTS221Sensor   *hum_temp;
static LSM6DSLSensor  *acc_gyro;
static LPS22HBSensor  *pressure;
static LSM303AGRMagSensor *mag;



static const char* connectionString = "HostName=iotc-9fb34c7f-5eb6-4b1a-be18-eae9abab68fd.azure-devices.net;DeviceId=bpd0gzmw17;SharedAccessKey=tRThrVckItU1N17T1gKnnrrdb+bmRyurJvGEl7cVhIs=";
 
static const char* deviceId               = "bpd0gzmw17"; /*must match the one on connectionString*/

// to report F uncomment this #define CTOF(x)         (((double)(x)*9/5)+32)
#define CTOF(x)         (x)

Thread azure_client_thread(osPriorityNormal, 8*1024, NULL, "azure_client_thread"); // @suppress("Type cannot be resolved")
Thread mag_sensor_thread(osPriorityHigh, 8*1024, NULL, "mag_sensor_thread");

static void azure_task(void);


//
// The mems sensor is setup to generate an interrupt with a tilt
// is detected at which time the blue LED is set to blink, also
// initialize all the ensors...
//

// static int tilt_event;

// void mems_int1(void)
// {
//     tilt_event++;
// }

void mems_init(void)
{
    // acc_gyro->attach_int1_irq(&mems_int1);  // Attach callback to LSM6DSL INT1
    // hum_temp->enable();                     // Enable HTS221 enviromental sensor
    // pressure->enable();                     // Enable barametric pressure sensor
    // acc_gyro->enable_x();                   // Enable LSM6DSL accelerometer
    // acc_gyro->enable_tilt_detection();      // Enable Tilt Detection
    
    mag->set_m_low_power_mode();  
}

void mag_sensor_ISR(void) {

    mag_sensor_thread.signal_set(GET_MAG_TAKE_MEASUREMENT);
}

void mag_reading_ISR(void) {

    mag_sensor_thread.signal_set(GET_MAG_READING_START);
} 

void get_mag_reading(void) {

    int32_t axisData[3];

    float32_t FIRstate[BLOCK_SIZE + NUM_TAPS - 1];
    arm_fir_instance_f32 FIRfilter;
    arm_fir_init_f32(&FIRfilter, NUM_TAPS, (float32_t *) &filterCoeffs[0], &FIRstate[0], BLOCK_SIZE);  

    mag_ISR_timer.attach(&mag_sensor_ISR, 0.0071);

    Timer t;

    while (true) {

        ThisThread::flags_wait_any(GET_MAG_TAKE_MEASUREMENT);
        // t.start();

        mag->take_m_single_measurement();
        mag->get_m_axes(axisData);
        magIn = sqrt(pow((float32_t)axisData[0], 2) + pow((float32_t)axisData[1], 2) + pow((float32_t)axisData[2], 2));

        arm_fir_f32(&FIRfilter, &magIn, &magOut, BLOCK_SIZE);
        magReadings.push_back( (float) magOut );

        if (magReadings.size() >= FILTER_BUFFER_SIZE) {
            
            float avg = 0;
            for (int i = 0; i < FILTER_BUFFER_SIZE; i++) {
                avg += magReadings[i];
            }
            printf("%f\n", avg/FILTER_BUFFER_SIZE);
            magReadings.clear();

            // if (!(magReadings.size() % 10)) {
            //     azure_client_thread.signal_set(AZURE_START);
            // }
        }

        // t.stop();
        // float t1 = t.read();
        // t.reset();
        // printf('t: %f\n', t1 );
    }   
}

// The main routine simply prints a banner, initializes the system
// starts the worker threads and waits for a termination (join)

int main(void)
{
    printf("\r\n");
    printf("     ****\r\n");
    printf("    **  **     Azure IoTClient Example, version %s\r\n", APP_VERSION);
    printf("   **    **    by AVNET\r\n");
    printf("  ** ==== **   \r\n");
    printf("\r\n");
    printf("The example program interacts with Azure IoTHub sending \r\n");
    printf("sensor data and receiving messeages (using ARM Mbed OS v5.x)\r\n");
    printf("->using %s Environmental Sensor\r\n", ENV_SENSOR);
    #ifdef IOTHUBTRANSPORTHTTP_H
        printf("->using HTTPS Transport Protocol\r\n");
    #else
        printf("->using MQTT Transport Protocol\r\n");
    #endif
    printf("\r\n");
    printf("This is the Nick Version 4\n");

    if (platform_init() != 0) {
       printf("Error initializing the platform\r\n");
       return -1;
       }


    XNucleoIKS01A2 *mems_expansion_board = XNucleoIKS01A2::instance(I2C_SDA, I2C_SCL, D4, D5);
    mag = mems_expansion_board->magnetometer;
    mems_init();

    mag_sensor_thread.start(get_mag_reading);
    azure_client_thread.start(azure_task);
    //mag_reading_timer.attach(&mag_reading_ISR, 1.0);

    mag_sensor_thread.join();
    azure_client_thread.join();
    platform_deinit();
    printf(" - - - - - - - ALL DONE - - - - - - - \n");
    return 0;
}

//
// This function sends the actual message to azure
//
//
// *************************************************************
//  AZURE STUFF...

char* makeMessage(IoTDevice* iotDev)
{
    static char buffer[80];
    const int   msg_size = 512;
    char*       ptr      = (char*)malloc(msg_size);
    time_t      rawtime;
    struct tm   *ptm;

    time(&rawtime);
    ptm = gmtime(&rawtime);
    strftime(buffer,80,"%a %F %X",ptm);
    iotDev->TOD = buffer;
    int c = (strstr(buffer,":")-buffer) - 2;
    printf("Send IoTHubClient Message@%s - ",&buffer[c]);
    snprintf(ptr, msg_size, IOTDEVICE_MSG_FORMAT,
                            iotDev->ObjectName,
                            iotDev->ObjectType,
                            iotDev->Version,
                            iotDev->ReportingDevice,
                            iotDev->MagneticField,
                            iotDev->ElectricField,
                            iotDev->Temperature,
                            iotDev->Humidity,
                            iotDev->Pressure,
                            iotDev->Tilt,
                            iotDev->ButtonPress,
                            iotDev->TOD);
    return ptr;
}


void sendMessage(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, char* buffer, size_t size)
{
    IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char*)buffer, size);
    if (messageHandle == NULL) {
        printf("unable to create a new IoTHubMessage\r\n");
        return;
        }
    if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messageHandle, NULL, NULL) != IOTHUB_CLIENT_OK)
        printf("FAILED to send! [RSSI=%d]\n", platform_RSSI());
    else
        printf("OK. [RSSI=%d]\n",platform_RSSI());

    IoTHubMessage_Destroy(messageHandle);
}


void azure_task(void)
{
    bool transmit = false;
 
    int  k;
    int  msg_sent=1;
    u8_t magStatus;

    //
    // setup the iotDev struction contents...
    //
    IoTDevice* iotDev = (IoTDevice*)malloc(sizeof(IoTDevice));

    if (iotDev == NULL) {
        printf("Failed to malloc space for IoTDevice\r\n");
        return;
    }

    iotDev->ObjectName      = (char*)"Avnet NUCLEO-L496ZG+BG96 Azure IoT Client";
    iotDev->ObjectType      = (char*)"SensorData";
    iotDev->Version         = (char*)APP_VERSION;
    iotDev->ReportingDevice = deviceId;
    iotDev->TOD             = (char*)"";
    iotDev->Temperature     = 0.0;
    iotDev->MagneticField   = 0;
    iotDev->ElectricField   = 0;
    iotDev->Humidity        = 0;
    iotDev->Pressure        = 0;
    iotDev->Tilt            = 0x2;
    iotDev->ButtonPress     = 0;

    // /* Setup IoTHub client configuration */
    // #ifdef IOTHUBTRANSPORTHTTP_H
    //     IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, HTTP_Protocol);
    // #else
    //     IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol);
    // #endif

    // if (iotHubClientHandle == NULL) {
    //     printf("Failed on IoTHubClient_Create\r\n");
    //     return;
    // }

    //     // add the certificate information
    // if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
    //     printf("failure to set option \"TrustedCerts\"\r\n");

    // #if MBED_CONF_APP_TELUSKIT == 1
    //     if (IoTHubClient_LL_SetOption(iotHubClientHandle, "product_info", "TELUSIOTKIT") != IOTHUB_CLIENT_OK)
    //         printf("failure to set option \"product_info\"\r\n");
    // #endif

    // #ifdef IOTHUBTRANSPORTHTTP_H
    //     // polls will happen effectively at ~10 seconds.  The default value of minimumPollingTime is 25 minutes.
    //     // For more information, see:
    //     //     https://azure.microsoft.com/documentation/articles/iot-hub-devguide/#messaging

    //     unsigned int minimumPollingTime = 9;
    //     if (IoTHubClient_LL_SetOption(iotHubClientHandle, "MinimumPollingTime", &minimumPollingTime) != IOTHUB_CLIENT_OK)
    //         printf("failure to set option \"MinimumPollingTime\"\r\n");
    // #endif

    //     if (IoTHubClientCore_LL_SetRetryPolicy(iotHubClientHandle, IOTHUB_CLIENT_RETRY_NONE, 1) != IOTHUB_CLIENT_OK){
    //         printf("failure to set retry option\n");
    //     }
    
    while (true) {

        ThisThread::flags_wait_any(AZURE_START);
        printf("Azure Thread!\n");
        // t2.stop();
        // printf("t:%d\n",(int)t2.read_us());
        // t2.reset();
        

        char*  msg;
        size_t msgSize;
        // int32_t magData[3];

        //hum_temp->get_temperature(&gtemp);           // get Temp
        //hum_temp->get_humidity(&ghumid);             // get Humidity
        //pressure->get_pressure(&gpress);             // get pressure
        
        // t.start();
        // mag->take_m_single_measurement();
        

        // // Wait while for the measurement to complete - Nick
        // while (magStatus != LSM303AGR_MAG_MD_IDLE2_MODE) {
        //     mag->get_m_mode_status(&magStatus);
        //     //printf("Status: %d", (int)magStatus);
        // }
        
        // mag->get_m_axes(magData);
        
        // iotDev->MagneticField = (int) sqrt(pow(magData[0], 2) + pow(magData[1], 2) + pow(magData[2], 2));

     // if( tilt_event ) {
        //     tilt_event = 0;
        //     iotDev->Tilt |= 1;
        // }
        // iotDev->Tilt &= 0x2;
        // iotDev->ButtonPress = 0;

        //iotDev->Temperature = CTOF(gtemp);
        //iotDev->Humidity    = (int)ghumid;
        //iotDev->Pressure    = (int)gpress;


        if (transmit) {
        
            printf("(%04d)",msg_sent++);
            msg = makeMessage(iotDev);
            msgSize = strlen(msg);
            //sendMessage(iotHubClientHandle, msg, msgSize);
            free(msg);

            /* schedule IoTHubClient to send events/receive commands */
            //IoTHubClient_LL_DoWork(iotHubClientHandle);
        }   

        //ThisThread::sleep_for(30000);  //in msec // @suppress("Function cannot be resolved")
        //printf("X:%d, Y:%d, Z:%d, RMS:%d, t:%f\n", (int)magData[0], (int)magData[1], (int)magData[2], iotDev->MagneticField, dt);

        //printf('%d, %d\n', sampleRate - (int)dt, (int)dt);
       
    }

    free(iotDev);
    // IoTHubClient_LL_Destroy(iotHubClientHandle);
    return;
}