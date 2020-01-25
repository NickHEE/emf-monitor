// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include "mbed.h"
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
#include "bg96gps.hpp"
#include "button.hpp"
#include "azure_certs.h"

Mutex magMutex;


#include "ST7735/ST7735.h" // from Rolland Kamp: https://os.mbed.com/users/rolo644u/code/ST7735/file/291ac9fb4d64/ST7735.cpp
                           //   modified to have functions suitable for this project
#include "screen_char_things.h"
ST7735* screen;
void string_to_screen(int line, int col, char *string_array, int str_length);
void num_to_screen(int line, int col, int magnitude, int data_length);
void single_char(int line, int col, const short *single_char);
DigitalOut   RST_pin(D8);
const int screen_update = 250;
Thread screen_thread(osPriorityNormal, 8*1024, NULL, "screen_thread");
static void screen_task(void);

int global_magnitude = 0;
int global_max_magnitude = 10;
int global_battery_val = 35;
char global_name[] = "Joseph";


/*
#include "lvgl/lvgl.h" // from littlevGL: https://littlevgl.com/download
#include "lv_examples-master\lv_tutorial\1_hello_world\lv_tutorial_hello_world.h"
#define LVGL_TICK 5
#define TICKER_TIME 0.001*LVGL_TICK
Ticker ticker;
//TS_StateTypeDef TS_State;
void display_init(void);
void lv_ticker_func();
static void my_disp_flush_cb(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p);
const int tick_interval = 500;
*/




#define APP_VERSION "1.2"
#define IOT_AGENT_OK CODEFIRST_OK
static const char* connectionString = "HostName=XXXX;DeviceId=xxxx;SharedAccessKey=xxxx";
static const char* deviceId         = "xxxx"; /*must match the one on connectionString*/
// to report F uncomment this #define CTOF(x)         (((double)(x)*9/5)+32)
#define CTOF(x)         (x)
Thread azure_client_thread(osPriorityNormal, 8*1024, NULL, "azure_client_thread");
static void azure_task(void);
/* LED Management */
DigitalOut   RED_led(LED1);
DigitalOut   BLUE_led(LED2);
DigitalOut   GREEN_led(LED3);
const int    blink_interval = 500; //msec
int          RED_state, BLUE_state, GREEN_state;
/* create the GPS elements for example program */
gps_data gdata; 
bg96_gps gps;   
#define GREEN       4  //0 0100 GREEN
#define BLUE        2  //0 0010
#define RED         1  //0 0001 RED
#define LED_ON      8  //0 1xxx
#define LED_BLINK  16  //1 xxxx
#define LED_OFF     0  //0 0xxx
#define SET_LED(l,s) (l##_led = ((l##_state=s)&LED_ON)? 1: 0)

// The LED thread simply manages the LED's on an on-going basis
static void screen_task(void)
{
    // draw magnetometer information to screen
   int magnitude = 0; // variable to simulate magnetometer measurements
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
        num_to_screen(row_for_mag,col_for_mag,global_magnitude,mag_length);
        num_to_screen(row_for_bat,col_for_bat+4,global_battery_val, bat_length);

        if (last_name != &global_name[0])
        {
            string_to_screen(row_for_name, col_for_name+3, global_name, 12);
            last_name = &global_name[0];
        }

        if (global_max_magnitude > old_global_max)
        {
           num_to_screen(row_for_max_mag,col_for_max_mag,global_max_magnitude,max_mag_length);
           old_global_max = global_max_magnitude;
        }
        magMutex.unlock();
        ThisThread::sleep_for(screen_update);  //in msec
        }
}



//
// The main routine simply prints a banner, initializes the system
// starts the worker threads and waits for a termination (join)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(void) /////////////////////////////////////////////////////////////////////////////////////////////////////////
{
    // for verification that the program was successfully uploaded/executed
    printf("\r\n");
    printf("     ****\r\n");
    printf("    **  **     Azure IoTClient Example, version %s\r\n", APP_VERSION);
    printf("   **    **    by AVNET\r\n");
    printf("  ** ==== **   \r\n\n");
    printf("Identifier #45\n");
    // end of initial program verification

    // initialize screen
    screen = new ST7735(D10, D9, D11, D13);
    RST_pin = 0; wait_ms(50);
    RST_pin = 1; wait_ms(50);
    screen->initR(INITR_GREENTAB);
    screen->setRotation(0);wait_ms(100);
    screen->fillScreen(ST7735_BLACK); // have as other color for testing purposes
    screen_thread.start(screen_task);
    printf("done initialization of screen\n");
    // done screen initialization
    

/*
    //azure_client_thread.start(azure_task);
    //azure_client_thread.join();
wait_ms(1000);
printf("attempting LittleVGL thing\n");

// things that need to be done for LittleVGL thing
    display_init(); // done earlier
    ticker.attach(callback(&lv_ticker_func),TICKER_TIME);
 
    lv_tutorial_hello_world(); 
*/

printf("made it to infinite loop");
    while(true)
    { }
    //LED_thread.terminate();

    printf(" - - - - - - - ALL DONE - - - - - - - \n");
 return 0;
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












/*

void display_init(void){
    lv_init();                                  //Initialize the LittlevGL
    static lv_disp_buf_t disp_buf;
    static lv_color_t buf[LV_HOR_RES_MAX * 10]; //Declare a buffer for 10 lines                                                              
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10); //Initialize the display buffer
    
    //Implement and register a function which can copy a pixel array to an area of your display
    lv_disp_drv_t disp_drv;                     //Descriptor of a display driver
    lv_disp_drv_init(&disp_drv);                //Basic initialization
    disp_drv.flush_cb = my_disp_flush_cb;       //Set your driver function
    disp_drv.buffer = &disp_buf;                //Assign the buffer to the display
    lv_disp_drv_register(&disp_drv);            //Finally register the driver
}

void lv_ticker_func(){
    lv_tick_inc(LVGL_TICK); 
    //Call lv_tick_inc(x) every x milliseconds in a Timer or Task (x should be between 1 and 10). 
    //It is required for the internal timing of LittlevGL.
    lv_task_handler(); 
    //Call lv_task_handler() periodically every few milliseconds. 
    //It will redraw the screen if required, handle input devices etc.
}

void my_disp_flush_cb(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p)
{
    //The most simple case (but also the slowest) to put all pixels to the screen one-by-one
    uint16_t x, y;
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            //put_px(x, y, *color_p)
            screen->drawPixel( x, y, color_p->full);
            color_p++;
        }
    }
    //IMPORTANT!!!* Inform the graphics library that you are ready with the flushing
    lv_disp_flush_ready(disp_drv);
}

*/











/*

// Button callbacks for a press and release (light an LED) 
static bool button_pressed = false;
void ub_press(void)
{
    button_pressed = true;
    SET_LED(RED,LED_ON);
}

void ub_release(int x)
{
    button_pressed = false;
    SET_LED(RED,LED_OFF);
}
*/

/*
// The LED thread simply manages the LED's on an on-going basis
static void LED_task(void)
{
    printf("inside LED_task\n");
    while (true) {

        if( GREEN_state & LED_OFF ) 
            GREEN_led = 0;
        else if( GREEN_state & LED_ON ) 
            GREEN_led = 1;
        else if( GREEN_state & LED_BLINK ) 
            GREEN_led = !GREEN_led;

        if( BLUE_state & LED_OFF ) 
            BLUE_led = 0;
        else if( BLUE_state & LED_ON ) 
            BLUE_led = 1;
        else if( BLUE_state & LED_BLINK ) 
            BLUE_led = !BLUE_led;
        
        if( RED_state & LED_OFF ) 
            RED_led = 0;
        else if( RED_state & LED_ON ) 
            RED_led = 1;
        else if( RED_state & LED_BLINK ) 
            RED_led = !RED_led;

        lv_tick_inc(blink_interval); //Tell LittelvGL that "tick_interval" milliseconds were elapsed
        ThisThread::sleep_for(blink_interval);  //in msec
        }
}
*/

/*

//
// This function sends the actual message to azure
//

// *************************************************************
//  AZURE STUFF...
//
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
                            iotDev->lat,
                            iotDev->lon,
                            iotDev->gpstime,
                            iotDev->gpsdate,
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

IOTHUBMESSAGE_DISPOSITION_RESULT receiveMessageCallback(
    IOTHUB_MESSAGE_HANDLE message, 
    void *userContextCallback)
{
    const unsigned char *buffer = NULL;
    size_t size = 0;

    if (IOTHUB_MESSAGE_OK != IoTHubMessage_GetByteArray(message, &buffer, &size))
    {
        return IOTHUBMESSAGE_ABANDONED;
    }

    // message needs to be converted to zero terminated string
    char * temp = (char *)malloc(size + 1);
    if (temp == NULL)
    {
        return IOTHUBMESSAGE_ABANDONED;
    }
    strncpy(temp, (char*)buffer, size);
    temp[size] = '\0';

    printf("Receiving message: '%s'\r\n", temp);
    if( !strcmp(temp,"led-blink") ) {
        SET_LED(GREEN,LED_BLINK);
        printf("start blinking\n");
        }
    if( !strcmp(temp,"led-on") ) {
        SET_LED(GREEN,LED_ON);
        printf("turn on\n");
        }
    if( !strcmp(temp,"led-off") ) {
        SET_LED(GREEN,LED_OFF);
        printf("turn off\n");
        }

    free(temp);

    return IOTHUBMESSAGE_ACCEPTED;
}

void azure_task(void)
{
    bool button_press = false, runTest = true;
    bool tilt_detection_enabled=true;
    float gtemp, ghumid, gpress;

    int  k;
    int  msg_sent=1;

    Button user_button(BUTTON1,Button::MBED_CONF_APP_BUTTON_ACTIVE_STATE, ub_release);
    user_button.setButton_press_cb(ub_press);

    SET_LED(RED,LED_ON);
    SET_LED(BLUE,LED_ON);
    SET_LED(GREEN,LED_ON);

   //Setup IoTHub client configuration 
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

    IoTDevice* iotDev = (IoTDevice*)malloc(sizeof(IoTDevice));
    if (iotDev == NULL) {
        printf("Failed to malloc space for IoTDevice\r\n");
        return;
        }

    // set C2D and device method callback
    IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, receiveMessageCallback, NULL);

    //
    // setup the iotDev struction contents...
    //
    iotDev->ObjectName      = (char*)"Avnet NUCLEO-L496ZG+BG96 Azure IoT Client";
    iotDev->ObjectType      = (char*)"SensorData";
    iotDev->Version         = (char*)APP_VERSION;
    iotDev->ReportingDevice = (char*)"STL496ZG-BG96";
    iotDev->TOD             = (char*)"";
    iotDev->Temperature     = 0.0;
    iotDev->lat             = 0.0;
    iotDev->lon             = 0.0;
    iotDev->gpstime         = 0.0;
    iotDev->Humidity        = 0;
    iotDev->Pressure        = 0;
    iotDev->Tilt            = 0x2;
    iotDev->ButtonPress     = 0;
    memset(iotDev->gpsdate,0x00,7);

    SET_LED(RED,LED_OFF);
    SET_LED(BLUE,LED_OFF);
    SET_LED(GREEN,LED_OFF);

    while (runTest) {
        char*  msg;
        size_t msgSize;

        gps.gpsLocation(&gdata);
        iotDev->lat = gdata.lat;
        iotDev->lon = gdata.lon;
        iotDev->gpstime = gdata.utc;
        memcpy(iotDev->gpsdate, gdata.date, 7);

#if MBED_CONF_APP_IKSVERSION == 2
        hum_temp->get_temperature(&gtemp);           // get Temp
        hum_temp->get_humidity(&ghumid);             // get Humidity
        pressure->get_pressure(&gpress);             // get pressure
#elif MBED_CONF_APP_IKSVERSION == 1
        CALL_METH(temp, get_temperature, &gtemp, 0.0f);
        CALL_METH(hum, get_humidity, &ghumid, 0.0f);
        CALL_METH(pressure, get_pressure, &gpress, 0.0f);
#else
        gtemp  = 0.0;
        ghumid = 0.0;
        gpress = 0.0;
#endif

        iotDev->Temperature = CTOF(gtemp);
        iotDev->Humidity    = (int)ghumid;
        iotDev->Pressure    = (int)gpress;

        if( tilt_event ) {
            tilt_event = 0;
            iotDev->Tilt |= 1;
            }

        iotDev->ButtonPress = button_pressed?1:0;
        if( user_button.chkButton_press(&k) ) {
            if( k > 3000 ) {
                printf("User Requested Termination (held button for %d msec), exiting.\n",k);
                runTest = false;
                }
            else{
                iotDev->ButtonPress = 1;
                tilt_detection_enabled = !tilt_detection_enabled;
#if MBED_CONF_APP_IKSVERSION == 2
                if( !tilt_detection_enabled ) {
                    acc_gyro->disable_tilt_detection();
                    iotDev->Tilt &= 1;
                    }
                else{
                    acc_gyro->enable_tilt_detection();
                    iotDev->Tilt |= 2;
                    }
#endif
                }
            }

        printf("(%04d)",msg_sent++);
        msg = makeMessage(iotDev);
        msgSize = strlen(msg);
        sendMessage(iotHubClientHandle, msg, msgSize);
        free(msg);
        SET_LED(BLUE,LED_OFF);
        iotDev->Tilt &= 0x2;
        iotDev->ButtonPress = 0;

        //schedule IoTHubClient to send events/receive commands 
        IoTHubClient_LL_DoWork(iotHubClientHandle);

#if defined(MBED_HEAP_STATS_ENABLED)
        mbed_stats_heap_t heap_stats; //jmf

        mbed_stats_heap_get(&heap_stats);
        printf("  Current heap: %lu\r\n", heap_stats.current_size);
        printf(" Max heap size: %lu\r\n", heap_stats.max_size);
        printf("     alloc_cnt:	%lu\r\n", heap_stats.alloc_cnt);
        printf("alloc_fail_cnt:	%lu\r\n", heap_stats.alloc_fail_cnt);
        printf("    total_size:	%lu\r\n", heap_stats.total_size);
        printf(" reserved_size:	%lu\r\n", heap_stats.reserved_size);
#endif 

#if defined(MBED_STACK_STATS_ENABLED)
        int cnt_ss = osThreadGetCount();
        mbed_stats_stack_t *stats_ss = (mbed_stats_stack_t*) malloc(cnt_ss * sizeof(mbed_stats_stack_t));
        
        cnt_ss = mbed_stats_stack_get_each(stats_ss, cnt_ss);
        for (int i = 0; i < cnt_ss; i++) 
            printf("Thread: 0x%lX, Stack size: %lu, Max stack: %lu\r\n", stats_ss[i].thread_id, stats_ss[i].reserved_size, stats_ss[i].max_size);
#endif 

#if defined(MBED_THREAD_STATS_ENABLED)
#define MAX_THREAD_STATS  10
            mbed_stats_thread_t *stats = new mbed_stats_thread_t[MAX_THREAD_STATS];
            int count = mbed_stats_thread_get_each(stats, MAX_THREAD_STATS);
            
            for(int i = 0; i < count; i++) {
                printf("ID: 0x%lx \n", stats[i].id);
                printf("Name: %s \n", stats[i].name);
                printf("State: %ld \n", stats[i].state);
                printf("Priority: %ld \n", stats[i].priority);
                printf("Stack Size: %ld \n", stats[i].stack_size);
                printf("Stack Space: %ld \n", stats[i].stack_space);
                printf("\n");
                }
#endif 
        ThisThread::sleep_for(5000);  //in msec
        }
    free(iotDev);
    IoTHubClient_LL_Destroy(iotHubClientHandle);
    return;
}
*/
