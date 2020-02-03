// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

//#define USE_MQTT
//py -2 -m mbed compile -m NUCLEO_L496ZG -t GCC_ARM --profile toolchain_debug.json

#include <stdlib.h>
#include <math.h>
#include <vector>
#include <numeric>

#include "mbed.h"
#include "arm_math.h"
#include "MMC5603NJ.h"

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
#include "azure_certs.h"

#include "ST7735/ST7735.h" // from Rolland Kamp: https://os.mbed.com/users/rolo644u/code/ST7735/file/291ac9fb4d64/ST7735.cpp
                           //   modified to have functions suitable for this project
#include "screen_char_things.h"

#define APP_VERSION "1.2"
#define IOT_AGENT_OK CODEFIRST_OK

#define MAG_TAKE_MEASUREMENT 1
#define MAG_IS_RECORDING 2
#define MAG_TAKE_1_MIN_AVERAGE 4

#define AZURE_TRANSMIT 1

float32_t magIn;
float32_t magOut;

std::vector<float> magSamples;
std::vector<float> magReadings;
std::vector<float> magAverages;

float magCurrent;
float magSessionMax;
float mag1minAvg;
float magSessionAvg;

Mutex magMutex;

Ticker mag_sample_ticker;
Ticker mag_1_min_avg_ticker;

ST7735* screen;
const int screen_update = 250;
DigitalOut   RST_pin(D8);
int global_battery_val = 35;
char global_name[] = "Joseph";

/* The following is the message we will be sending to Azure */
typedef struct IoTDevice_t {
    char* ObjectName;
    char* ObjectType;
    char* Version;
    const char* ReportingDevice;
    int   MagneticField;
    int   ElectricField;
    char* UserID;
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
     "\"UserID\":\"%s\","          \
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

Thread azure_client_thread(osPriorityNormal, 8*1024, NULL, "azure_client_thread"); // @suppress("Type cannot be resolved")
Thread mag_sensor_thread(osPriorityHigh, 8*1024, NULL, "mag_sensor_thread");
Thread UI_thread(osPriorityNormal, 8*1024, NULL, "UI_thread");

static void azure_task(void);



void string_to_screen(int line, int col, char *string_array, int str_length);
void num_to_screen(int line, int col, int magnitude, int data_length);
void single_char(int line, int col, const short *single_char);

void mems_init(void) {
 
    mag->set_m_low_power_mode();  
}

void mag_sample_ISR(void) {

    mag_sensor_thread.signal_set(MAG_TAKE_MEASUREMENT);
}

void mag_recording_start_ISR(void) {
}

void get_mag_reading(void) {

    int32_t axisData[3];

    float32_t FIRstate[BLOCK_SIZE + NUM_TAPS - 1];
    arm_fir_instance_f32 FIRfilter;
    arm_fir_init_f32(&FIRfilter, NUM_TAPS, (float32_t *) &filterCoeffs[0], &FIRstate[0], BLOCK_SIZE);  

    mag_sample_ticker.attach(&mag_sample_ISR, 0.0071);


    while (true) {

        ThisThread::flags_wait_any(MAG_TAKE_MEASUREMENT);

        mag->take_m_single_measurement();
        mag->get_m_axes(axisData);
        magIn = sqrt(pow((float32_t)axisData[0], 2) + pow((float32_t)axisData[1], 2) + pow((float32_t)axisData[2], 2));

        arm_fir_f32(&FIRfilter, &magIn, &magOut, BLOCK_SIZE);
        magSamples.push_back( (float) magOut );

        if (magSamples.size() >= FILTER_BUFFER_SIZE) {
            
            magMutex.lock();

            magCurrent = std::accumulate(magSamples.begin(), magSamples.end(), 0.0) / FILTER_BUFFER_SIZE;
            printf("%f\n", magCurrent);

            if (ThisThread::flags_get() & MAG_IS_RECORDING) {
                magReadings.push_back(magCurrent);
                if (magCurrent > magSessionMax) {
                    magSessionMax = magCurrent;
                }
            }

            magMutex.unlock();
            magSamples.clear();    
        }

        if (ThisThread::flags_get() & MAG_TAKE_1_MIN_AVERAGE) {
            
            mag1minAvg = std::accumulate(magReadings.begin(), magReadings.end(), 0.0) / magReadings.size();
        
            magMutex.lock();
            // Maybe we should just store all the 1 min averages of a session
            magSessionAvg = (magSessionAvg + mag1minAvg) / 2;
            magMutex.unlock();
            magReadings.clear();

            azure_client_thread.signal_set(AZURE_TRANSMIT);
        }
    }   
}

static void screen_task(void)
{
    // draw magnetometer information to screen
   int magnitude = 0; // variable to simulate magnetometer measurements
   int max_magnitude = 0;
   int row_for_mag = 8;
   int col_for_mag = 9;
   int mag_length = 5;
   string_to_screen(row_for_mag, col_for_mag-8, "CURRENT:", 8);
   num_to_screen(row_for_mag,col_for_mag,magnitude,mag_length);
   single_char(row_for_mag,col_for_mag+mag_length,&m[0][0]);
   single_char(row_for_mag,col_for_mag+mag_length+1,&G_[0][0]);

    // draw max magnetometer information to screen
   int row_for_max_mag = 9;
   int col_for_max_mag = 9;
   int max_mag_length = 5;
   int old_global_max = 0;
   string_to_screen(row_for_max_mag, col_for_max_mag-4, "MAX:", 4);
   num_to_screen(row_for_max_mag,col_for_max_mag,old_global_max,max_mag_length);
   single_char(row_for_max_mag,col_for_max_mag+mag_length,&m[0][0]);
   single_char(row_for_max_mag,col_for_max_mag+mag_length+1,&G_[0][0]);


   // draw battery information to screen
   int battery_value = 98;
   int row_for_bat = 1;
   int col_for_bat = 1;
   int bat_length = 3;
   string_to_screen(row_for_bat, col_for_bat, "BAT:", 4);
   num_to_screen(row_for_bat,col_for_bat+4,battery_value, bat_length);
   single_char(row_for_bat,col_for_bat+bat_length+4,&percent[0][0]);

   // draw name information to screen
   char name[] = "Nicholas";
   char *last_name = name;
   int row_for_name = 2;
   int col_for_name = 1;
   string_to_screen(row_for_name, col_for_name, "ID:", 3);
   string_to_screen(row_for_name, col_for_name+3, name, 12);

   // draw LTE connection information to screen
   int row_for_LTE = 1;
   int col_for_LTE = 15;
   string_to_screen(row_for_LTE, col_for_LTE, "LTE:", 4);
   single_char(row_for_LTE, col_for_LTE+4, &LTE_3bar[0][0]);

   while (true) {
       // update values on screen
        magMutex.lock();
        magnitude = (int) magCurrent;
        max_magnitude = (int) magSessionMax;
        magMutex.unlock();

        num_to_screen(row_for_mag,col_for_mag,magnitude,mag_length);
        num_to_screen(row_for_bat,col_for_bat+4,global_battery_val, bat_length);

        if (last_name != &global_name[0])
        {
            string_to_screen(row_for_name, col_for_name+3, global_name, 12);
            last_name = &global_name[0];
        }

        if (max_magnitude > old_global_max)
        {
           num_to_screen(row_for_max_mag,col_for_max_mag,max_magnitude,max_mag_length);
           old_global_max = max_magnitude;
        }
        
        ThisThread::sleep_for(screen_update);  //in msec
        }
}

void single_char(int line, int col, const short *single_char)
{
    screen->drawOneChar(SPACING + col*(SPACING + CHAR_COL),  SPACING + line*(SPACING + CHAR_ROW), single_char);
}


// line/col = positioning of text box on screen (line = y axis, col = x axis)
// magnitude = length of value to write to screen
// data_length = number of digits (max) you expect this number to occupy e.g. a value from 0 - 100 requires data_length = 3
//      NOTE: max data_length is 10 digits
void num_to_screen(int line, int col, int magnitude, int data_length)
{
    static const int max_data_len = 10;
    const short *data[max_data_len];
    long max_val = 1;
    for (int i = 0; i < data_length; i++)
    {
        max_val*=10;
    }
      
    // data sanitization (only positive numbers allowed for now)
    if (magnitude < 0)
        magnitude = 0;
    if (magnitude >= max_val)
        magnitude = max_val-1;
    if (line < 0)
        line = 0;
    if (col < 0)
        col = 0;

    // split data into array, separated by digit position
    for (int i = (10 - 1); i >= 0; i--)
        {
        data[i] = numbers[magnitude % 10];
        magnitude /= 10;
        }

      // put digits on led_display, mG
      for (int i = (max_data_len-data_length); i < max_data_len; i++)
         {
         screen->drawOneChar(SPACING + col*(SPACING + CHAR_COL),  SPACING + line*(SPACING + CHAR_ROW), data[i]);
         col++;
         }
         
}

// line/col = positioning of text box on screen (line = y axis, col = x axis)
// string_array = string to display on screen. Can handle alpha-numeric characters and some special characters
//     charcters it does not know will be skipped
void string_to_screen(int line, int col, char *string_array, int string_length)
   {
       int index = 0;
       int letter_val;
       bool is_letter;
    while(string_array[index] != '\0')
    {
        is_letter = 1;
        letter_val = string_array[index];

        // handle lowercase values
        if ((string_array[index] >= 'a') && (string_array[index] <= 'z'))
           letter_val -= 'a'; // offset needed to convert lowercase to uppercase in ASCII

        // handle uppercase values
        else if ((string_array[index] >= 'A') && (string_array[index] <= 'Z'))
            letter_val -= 'A'; // offset needed to zero-index uppercase letters (e.g. 'A' = index of 0)
        
        // handle numbers
        else if ((string_array[index] >= '0') && (string_array[index] <= '9'))
        {
            letter_val -= '0'; // offset needed to zero-index numbers
            is_letter = 0;
        }        

        // handle special characters
        if ((string_array[index] == ' '))
        {
            screen->drawOneChar(SPACING + col*(SPACING + CHAR_COL),  SPACING + line*(SPACING + CHAR_ROW), &space[0][0]);
            index++;
            col++;
            continue;
        } else if ((string_array[index] == ':'))
        {
            screen->drawOneChar(SPACING + col*(SPACING + CHAR_COL),  SPACING + line*(SPACING + CHAR_ROW), &colon[0][0]);
            index++;
            col++;
            continue;
        }

        // discard other values
        if (letter_val >= 26)
        {
            index++;
            continue; // something that will break program
        }
        
        // draw alpha-numeric characters, then increment index (to access next character) and col (to move where next char will be drawn)
        if (is_letter == 1)
            screen->drawOneChar(SPACING + col*(SPACING + CHAR_COL),  SPACING + line*(SPACING + CHAR_ROW), letters[letter_val]);
        else 
            screen->drawOneChar(SPACING + col*(SPACING + CHAR_COL),  SPACING + line*(SPACING + CHAR_ROW), numbers[letter_val]);
        index++;
        col++;
    }   

    // fill remainder of field in blank space
    while (index < string_length)
    {
    screen->drawOneChar(SPACING + col*(SPACING + CHAR_COL),  SPACING + line*(SPACING + CHAR_ROW), &space[0][0]);
    index++;
    col++;
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

    // if (platform_init() != 0) {
    //    printf("Error initializing the platform\r\n");
    //    return -1;
    //    }

    XNucleoIKS01A2 *mems_expansion_board = XNucleoIKS01A2::instance(I2C_SDA, I2C_SCL, D4, D5);
    mag = mems_expansion_board->magnetometer;
    //mems_init();

    // screen = new ST7735(D10, D9, D11, D13);
    // RST_pin = 0; wait_ms(50);
    // RST_pin = 1; wait_ms(50);
    // screen->initR(INITR_GREENTAB);
    // screen->setRotation(0);wait_ms(100);
    // screen->fillScreen(ST7735_BLACK); // have as other color for testing purposes

    // UI_thread.start(screen_task);
    // mag_sensor_thread.start(get_mag_reading);
    // azure_client_thread.start(azure_task);

    // mag_sensor_thread.join();
    // azure_client_thread.join();
    // UI_thread.join();

    //platform_deinit();

    I2C i2c(I2C_SDA, I2C_SCL);
    MMC5603NJ magSensor = MMC5603NJ(&i2c, 140, CTRL_1_BW_150HZ);
    int test;

    float32_t FIRstate[BLOCK_SIZE + NUM_TAPS - 1];
    arm_fir_instance_f32 FIRfilter;
    arm_fir_init_f32(&FIRfilter, NUM_TAPS, (float32_t *) &filterCoeffs[0], &FIRstate[0], BLOCK_SIZE);

    float32_t magI;
    float32_t magO;  

    magSensor.startContinuousMode();
    while (true) {
        magI = magSensor.getMeasurement();
        arm_fir_f32(&FIRfilter, &magI, &magO, BLOCK_SIZE);
        printf("%f\n", magO);
    }

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
                            iotDev->UserID,
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
    iotDev->MagneticField   = 0;
    iotDev->ElectricField   = 0;
    iotDev->UserID          = (char*)"ILOVEBOBGILL69";

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

        ThisThread::flags_wait_any(AZURE_TRANSMIT);
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