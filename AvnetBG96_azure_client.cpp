// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define USE_MQTT
//py -2 -m mbed compile -m NUCLEO_L496ZG -t GCC_ARM --profile toolchain_debug.json

#include <stdlib.h>
#include <math.h>
#include <vector>
#include <numeric>

#include "mbed.h"
#include "arm_math.h"
#include "MMC5603NJ.h"
#include "lvgl/lvgl.h"
#include "ST7735/ST7735.h"

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

#define IOT_AGENT_OK CODEFIRST_OK
#define APP_VERSION "1.2"

#define MAG_TAKE_MEASUREMENT 1
#define MAG_IS_RECORDING 2
#define MAG_TAKE_1_MIN_AVERAGE 4
#define MAG_AZURE_READY 8

#define AZURE_TRANSMIT 1

#define LVGL_TICK 5
#define TICKER_TIME 0.001*LVGL_TICK

float32_t magIn;
float32_t magOut;

std::vector<float> magSamples;
std::vector<float> magReadings;
std::vector<float> magAverages;

float32_t magCurrent;
float magSessionMax;
float mag1minAvg;
float magSessionAvg;
int global_battery_val = 35;
char global_name[] = "Joseph";

lv_obj_t * current_EMF_val;
lv_obj_t * max_EMF_val;
lv_obj_t * battery_val;
lv_obj_t * name_val;
ST7735* screen;

Mutex magMutex;

Ticker mag_sample_ticker;
Ticker mag_1_min_avg_ticker;
Ticker ticker; //Joseph
const int tick_interval = 500;

DigitalOut   RST_pin(D8);
DigitalOut   cePin(D10);
DigitalOut   dcPin(D9);
DigitalOut   mosiPin(D11);
DigitalOut   sclkPin(D13);
SPI* spi = new SPI(D11, NC, D13); 

/* The following is the message we will be sending to Azure */
typedef struct IoTDevice_t {
    char* ObjectName;
    char* ObjectType;
    char* Version;
    const char* ReportingDevice;
    float   MagneticField;
    float   ElectricField;
    char* UserID;
    char* TOD;
} IoTDevice;

#define IOTDEVICE_MSG_FORMAT       \
   "{"                             \
     "\"ObjectName\":\"%s\","      \
     "\"ObjectType\":\"%s\","      \
     "\"Version\":\"%s\","         \
     "\"ReportingDevice\":\"%s\"," \
     "\"MagneticField\":\"%f\","   \
     "\"ElectricField\":\"%f\","   \
     "\"UserID\":\"%s\","          \
     "\"TOD\":\"%s UTC\""          \
   "}"

static const char* connectionString = "HostName=iotc-9fb34c7f-5eb6-4b1a-be18-eae9abab68fd.azure-devices.net;DeviceId=a77xgvjh8j;SharedAccessKey=Wgpc6I7FLAymepinjQ9HTgT/PdnPKZirI+pYi4pC6bU=";
static const char* deviceId               = "a77xgvjh8j"; /*must match the one on connectionString*/

Thread azure_client_thread(osPriorityNormal, 8*1024, NULL, "azure_client_thread"); // @suppress("Type cannot be resolved")
Thread mag_sensor_thread(osPriorityHigh, 8*1024, NULL, "mag_sensor_thread");
Thread ticker_thread(osPriorityNormal, 8*1024, NULL, "ticker_thread");

static void azure_task(void);
void get_mag_reading(void);
void display_init(void);

static void ticker_task(void);
static void btn_event_cb(lv_obj_t * btn, lv_event_t event);
static void my_disp_flush_cb(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p);
void main_screen(void);
static void label_refresher_task(void * p);

void mag_sample_ISR(void) {

    mag_sensor_thread.signal_set(MAG_TAKE_MEASUREMENT);
}

void mag_average_ISR(void) {

    mag_sensor_thread.signal_set(MAG_TAKE_1_MIN_AVERAGE);
}

void mag_recording_start_ISR(void) {
}

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

int main(void)
{
    printf("\r\n");
    printf("__________                                     __                       \n");
    printf("\\______   \\ ____ _____________    ____ _____ _/  |_  ___________  ______\n");
    printf("|       _// __ \\\\___   /\\__  \\  /    \\__  \\   __\\/  _ \\_  __ \\/  ___/\n");
    printf("|    |   \\  ___/ /    /  / __ \\|   |  \\/ __ \\|  | (  <_> )  | \\/\\___\\\n");
    printf("|____|_  /\\___  >_____ \\(____  /___|  (____  /__|  \\____/|__|  /____ >\n");
    printf("        \\/     \\/      \\/     \\/     \\/     \\/                       \\/ \n");
    printf("\r\n");
    printf("\r\n");
    printf("EMF MONITOR PROTOTYPE VERSION 1.0\n");
    printf("\r\n");
    
    // Init Display
    printf("Initializing Display...\n");
    screen = new ST7735(D10, D9, D11, D13);
    RST_pin = 0; wait_ms(50);
    RST_pin = 1; wait_ms(50);
    screen->initR(INITR_GREENTAB);
    screen->setRotation(0);wait_ms(100);
    screen->fillScreen(ST7735_BLACK); // have as other color for testing purposes
    display_init(); 
    main_screen();

    // Start Threads
    azure_client_thread.start(azure_task);
    mag_sensor_thread.start(get_mag_reading);
    mag_sensor_thread.signal_set(MAG_IS_RECORDING);
    ticker_thread.start(ticker_task);
    lv_task_create(label_refresher_task, 100, LV_TASK_PRIO_MID, NULL);
    
    mag_sensor_thread.join();
    azure_client_thread.join();
    ticker_thread.join();

    platform_deinit();

    return 0;
}

void get_mag_reading(void) {

    I2C i2c(I2C_SDA, I2C_SCL);
    MMC5603NJ magSensor = MMC5603NJ(&i2c, 140, CTRL_1_BW_255HZ);

    // Initialize ARM FIR filter object
    float32_t FIRstate[BLOCK_SIZE + NUM_TAPS - 1];
    arm_fir_instance_f32 FIRfilter;
    arm_fir_init_f32(&FIRfilter, NUM_TAPS, (float32_t *) &filterCoeffs[0], &FIRstate[0], BLOCK_SIZE);  

    ThisThread::flags_wait_any(MAG_AZURE_READY);

    mag_sample_ticker.attach(&mag_sample_ISR, 0.0071);
    mag_1_min_avg_ticker.attach(&mag_average_ISR, 60);

    while (true) {

        // Wait for the mag_sample_ticker to trigger
        ThisThread::flags_wait_any(MAG_TAKE_MEASUREMENT);

        // Start a measurement and wait for it to complete.
        // BW: 0x00 - 6.6ms, 0x01 - 3.5ms, 0x02 - 2.0ms, 0x03 - 1.2ms
        magSensor.takeMeasurement();
        ThisThread::sleep_for(T_WAIT_BW_150);
        magIn = magSensor.getMeasurement(false);

        // Filter the mag data to isolate the 60Hz component
        arm_fir_f32(&FIRfilter, &magIn, &magOut, BLOCK_SIZE);
        magSamples.push_back( (float) magOut );

        // Buffer ~3 cycles of 60Hz data and calculate the RMS value
        if (magSamples.size() >= FILTER_BUFFER_SIZE) {
            
            magMutex.lock();

            // Jank - change this later
            float* samplePtr = &magSamples[0];
            float32_t samples[FILTER_BUFFER_SIZE];
            std::copy(samplePtr, samplePtr+FILTER_BUFFER_SIZE, samples);

            arm_rms_f32(samples, FILTER_BUFFER_SIZE, &magCurrent);
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

            ThisThread::flags_clear(MAG_TAKE_1_MIN_AVERAGE);
            azure_client_thread.signal_set(AZURE_TRANSMIT);
        }
    }   
}

void display_init(void){
    lv_init();                                  //Initialize the LittlevGL
    static lv_disp_buf_t disp_buf;
    static lv_color_t buf[LV_HOR_RES_MAX * 10]; //Declare a buffer for 10 lines                                                              
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10); //Initialize the display buffer
    
    //Implement and register a function which can copy a pixel array to an area of your display
    lv_disp_drv_t disp_drv;                     //Descriptor of a display driver
    lv_disp_drv_init(&disp_drv);                //Basic initialization
    disp_drv.buffer = &disp_buf;                //Assign the buffer to the display
    disp_drv.flush_cb = my_disp_flush_cb;       //Set your driver function
    lv_disp_t *disp;
    disp = lv_disp_drv_register(&disp_drv);     //Finally register the driver
}

static void ticker_task(void)
{
    while(1)
    {
        // printf("tick %d\n", counter++);
       
        lv_tick_inc(LVGL_TICK); 
        //Call lv_tick_inc(x) every x milliseconds in a Timer or Task (x should be between 1 and 10). 
        //It is required for the internal timing of LittlevGL.
        lv_task_handler(); 
        //Call lv_task_handler() periodically every few milliseconds. 
        //It will redraw the screen if required, handle input devices etc.
        ThisThread::sleep_for(LVGL_TICK);  //in msec
    }
}

static void label_refresher_task(void * p)
{
    //printf("ping %d\n", global_magnitude++);
    //global_magnitude++;

    static int local_magnitude;
    static int local_max_magnitude;
    static int local_battery_val;
    static char * local_name;

    // only update screen if previous values get changed
    static uint32_t prev_value_current_EMF = 0;
    static uint32_t prev_value_max_EMF = 0;
    static uint32_t prev_value_bat = 0;
    static char * prev_value_name = &global_name[0];
    static char buf[32];

// make local copy of variables
    magMutex.lock();
    local_magnitude = (int) magCurrent;
    local_max_magnitude = (int) magSessionMax;
    local_battery_val = global_battery_val;
    local_name = &global_name[0];
    magMutex.unlock();

    // update present EMF value
    if(prev_value_current_EMF != local_magnitude) {

        if(lv_obj_get_screen(current_EMF_val) == lv_scr_act()) {
            sprintf(buf, "%d", local_magnitude);
            lv_label_set_text(current_EMF_val, buf);
        }
        prev_value_current_EMF = local_magnitude;
    }
    
    // update max EMF value
    if(prev_value_max_EMF != local_max_magnitude) {

        if(lv_obj_get_screen(current_EMF_val) == lv_scr_act()) {
            sprintf(buf, "%d", local_max_magnitude);
            lv_label_set_text(max_EMF_val, buf);
        }
        prev_value_max_EMF = local_max_magnitude;
    }

    // update battery % value
    if(prev_value_bat != local_battery_val) {

        if(lv_obj_get_screen(battery_val) == lv_scr_act()) {
            sprintf(buf, "%d", local_battery_val);
            lv_label_set_text(battery_val, buf);
        }
        prev_value_bat = local_battery_val;
    }

    // update employee name
    if(prev_value_name != &global_name[0]) {

        if(lv_obj_get_screen(name_val) == lv_scr_act()) {
            sprintf(buf, "%s", global_name);
            lv_label_set_text(name_val, buf);
        }
        prev_value_name = &global_name[0];
    }
}

void main_screen(void)
{
    lv_obj_t * scr = lv_disp_get_scr_act(NULL);     /*Get the current screen*/

	static lv_style_t style_new;                         /*Styles can't be local variables*/
    lv_style_copy(&style_new, &lv_style_pretty);         /*Copy a built-in style as a starting point*/
	style_new.text.font = &lv_font_roboto_12;

	static lv_style_t style_big_font;
    lv_style_copy(&style_big_font, &lv_style_pretty);
    style_big_font.text.font = &lv_font_roboto_22;
 

    /* Align the Label to the center
     * NULL means align on parent (which is the screen now)
     * 0, 0 at the end means an x, y offset after alignment*/
    //lv_obj_align(current_EMF_val, NULL, LV_ALIGN_CENTER, 0, 0);

	lv_obj_t * label;
    lv_obj_t * current_EMF_label;
    lv_obj_t * max_EMF_label;
    lv_obj_t * battery_label;
    lv_obj_t * name_label;

    // label = support text for value e.g. "EMF: "
    // val = value itself             e.g. "144"
    current_EMF_val =  lv_label_create(scr, NULL);
    lv_obj_set_style(current_EMF_val, &style_big_font);   
    current_EMF_label =  lv_label_create(scr, NULL); 
    lv_obj_set_style(current_EMF_label, &style_big_font);

    max_EMF_val =  lv_label_create(scr, NULL);
    lv_obj_set_style(max_EMF_val, &style_new); 
    max_EMF_label =  lv_label_create(scr, NULL); 
    lv_obj_set_style(max_EMF_label, &style_new);

    battery_val =  lv_label_create(scr, NULL);
    lv_obj_set_style(battery_val, &style_new); 
    battery_label =  lv_label_create(scr, NULL); 
    lv_obj_set_style(battery_label, &style_new);

    name_val =  lv_label_create(scr, NULL);
    lv_obj_set_style(name_val, &style_new); 
    name_label =  lv_label_create(scr, NULL); 
    lv_obj_set_style(name_label, &style_new);


// place location of main menu text items:
// current EMF value
    lv_label_set_text(current_EMF_label, "                mG");  
    lv_obj_set_y(current_EMF_label, 25);
    lv_obj_set_x(current_EMF_label, 1);

    lv_label_set_text(current_EMF_val, "00000");
	lv_obj_align(current_EMF_val, current_EMF_label, LV_ALIGN_OUT_BOTTOM_RIGHT, -35, -26);
    lv_label_set_align(current_EMF_val,LV_LABEL_ALIGN_RIGHT);

// maximum EMF value
    lv_label_set_text(max_EMF_label, "Max:               mG");  
    lv_obj_set_y(max_EMF_label, 46);
    lv_obj_set_x(max_EMF_label, 1);

    lv_label_set_text(max_EMF_val, "00000");
	lv_obj_align(max_EMF_val, max_EMF_label, LV_ALIGN_OUT_BOTTOM_RIGHT, -20, -14);
    lv_label_set_align(max_EMF_val,LV_LABEL_ALIGN_RIGHT);

// Name
    lv_label_set_text(name_label, "ID: ");  
    lv_obj_set_y(name_label, 13);
    lv_obj_set_x(name_label, 1);

    lv_label_set_text(name_val, "Joseph");
	lv_obj_align(name_val, name_label, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    //lv_label_set_align(name_val,LV_LABEL_ALIGN_RIGHT);

// Battery
    lv_label_set_text(battery_label, LV_SYMBOL_BATTERY_3 "        %" );  
    lv_obj_set_y(battery_label, 1);
    lv_obj_set_x(battery_label, 1);

    lv_label_set_text(battery_val, "100");
	lv_obj_align(battery_val, battery_label, LV_ALIGN_OUT_RIGHT_MID, -32, 0);
    //lv_label_set_align(battery_val,LV_LABEL_ALIGN_RIGHT);
    
   
    lv_obj_t * btn1 = lv_btn_create(lv_disp_get_scr_act(NULL), NULL);           /*Create a button on the currently loaded screen*/
    lv_obj_set_event_cb(btn1, btn_event_cb);                                    /*Set function to be called when the button is released*/
    lv_obj_align(btn1, NULL, LV_ALIGN_CENTER, 0, 0);                            /*Align below the label*/
    lv_obj_set_style(btn1, &style_new);

    label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Menu");
    lv_btn_set_toggle(btn1, true);
    lv_obj_set_y(btn1, 95);
    lv_obj_set_x(btn1, 1);
    lv_obj_set_size(btn1, 45,17);
    //lv_btn_toggle(btn1);

/*
    lv_obj_t * obj3;
    obj3 = lv_obj_create(scr, NULL);
    lv_obj_set_pos(obj3, 10, 105);
	lv_obj_set_size(obj3, 45,17);
    lv_obj_set_style(obj3, &style_new);
    // Add a label to the object.
     // Labels by default inherit the parent's style 
    label = lv_label_create(obj3, NULL);
    lv_label_set_text(label, "Menu");
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
    */
}

/**
 * Called when a button is released
 * @param btn pointer to the released button
 * @param event the triggering event
 * @return LV_RES_OK because the object is not deleted in this function
 */
static void btn_event_cb(lv_obj_t * btn, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        /*Increase the button width*/
        //lv_coord_t width = lv_obj_get_width(btn);
        //lv_obj_set_width(btn, width + 20);
        printf("Clicked\n");
    }
    else if (event == LV_EVENT_VALUE_CHANGED)
    {
        printf("Toggled\n");
    }
}

void my_disp_flush_cb(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p)
{
    //The most simple case (but also the slowest) to put all pixels to the screen one-by-one
    uint16_t x = area->x1; 
    uint16_t y = area->y1;
    uint8_t hi, lo;
    screen->setAddrWindow(x, y, x+(area->x2-area->x1), y+(area->y2-area->y1));
    // screen->dc->write(1);
    // screen->ce->write(0);
    dcPin = 1;
    cePin = 0;

    //wait_ms(10);
 
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            //put_px(x, y, *color_p)
            hi = color_p->full >> 8;
		    lo = color_p->full;
		
            // screen->spi->write(hi);
            // screen->spi->write(lo);
            spi->write(hi);
            spi->write(lo);


            //screen->drawPixel( x, y, color_p->full);
            color_p++;
        }
    }
    // screen->ce->write(1);
    cePin.write(1);
    //IMPORTANT!!!* Inform the graphics library that you are ready with the flushing
    lv_disp_flush_ready(disp_drv);
}

void azure_task(void)
{

    if (platform_init() != 0) {
       printf("Error initializing the platform\r\n");
       return;
    }

    bool transmit = true;
    int  msg_sent=1;

    // setup the iotDev struction contents...
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
    iotDev->MagneticField   = 1.0;
    iotDev->ElectricField   = 1.0;
    iotDev->UserID          = (char*)global_name;

    /* Setup IoTHub client configuration */
    #ifdef IOTHUBTRANSPORTHTTP_H
        IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, HTTP_Protocol);
    #else
        IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol);
    #endif

    if (iotHubClientHandle == NULL) {
        printf("Failed on IoTHubClient_Create\r\n");
        return;
    }

        // add the certificate information
    if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
        printf("failure to set option \"TrustedCerts\"\r\n");

    #if MBED_CONF_APP_TELUSKIT == 1
        if (IoTHubClient_LL_SetOption(iotHubClientHandle, "product_info", "TELUSIOTKIT") != IOTHUB_CLIENT_OK)
            printf("failure to set option \"product_info\"\r\n");
    #endif

    #ifdef IOTHUBTRANSPORTHTTP_H
        // polls will happen effectively at ~10 seconds.  The default value of minimumPollingTime is 25 minutes.
        // For more information, see:
        //     https://azure.microsoft.com/documentation/articles/iot-hub-devguide/#messaging

        unsigned int minimumPollingTime = 9;
        if (IoTHubClient_LL_SetOption(iotHubClientHandle, "MinimumPollingTime", &minimumPollingTime) != IOTHUB_CLIENT_OK)
            printf("failure to set option \"MinimumPollingTime\"\r\n");
    #endif

        if (IoTHubClientCore_LL_SetRetryPolicy(iotHubClientHandle, IOTHUB_CLIENT_RETRY_NONE, 1) != IOTHUB_CLIENT_OK){
            printf("failure to set retry option\n");
        }

    //mag_sensor_thread.signal_set(MAG_AZURE_READY);
    
    while (true) {
        printf("Azure TX!\n");
        ThisThread::flags_wait_any(AZURE_TRANSMIT);
        
        
        char*  msg;
        size_t msgSize;
        
        // iotDev->MagneticField = (int) sqrt(pow(magData[0], 2) + pow(magData[1], 2) + pow(magData[2], 2));

        if (transmit) {
        
            printf("(%04d)",msg_sent++);
            msg = makeMessage(iotDev);
            msgSize = strlen(msg);
            sendMessage(iotHubClientHandle, msg, msgSize);
            //printf(msg);
            free(msg);
            
            /* schedule IoTHubClient to send events/receive commands */
            IoTHubClient_LL_DoWork(iotHubClientHandle);
        }
    }

    free(iotDev);
    IoTHubClient_LL_Destroy(iotHubClientHandle);
    return;
}