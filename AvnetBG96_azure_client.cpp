// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// python -m mbed compile -m NUCLEO_L496ZG -t GCC_ARM --profile toolchain_debug.json

// Program used for developing UI system. Integration with other modules limited.
//  UI elements included: button input with debouncing, screen driver setup, LittleVGL (GUI library) menu setup
// Current setup is attempt at getting new screen to work with NUCLEO, using basic LittleVGL setup things
// Setup for when ST7735 screen with more complete GUI system was used commented out, found after code for simplified setup
//  Further explanation of how UI system currently works after code for simplified setup, just before ST7735 setup

// Things to do:
/*
 - get new screen to work with microcontroller/LittleVGL
 - finish functionality of remaining menu items (change units, change field, factory reset)
 - integrate menu functionality (e.g. flags) with relevant module(s)
 - add/integrate minor UI elements (e.g. add backlight/power buttons, add 4th navigation button if needed, add RGB LED and buzzer)
 - cleanup spaghetti code
*/



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
#include <string> 

//#include "lv_drivers-master\display\ST7565.h"
//#include "drivers/display/ST7565.h"
//#include "ST7735/ST7735.h" // from Rolland Kamp: https://os.mbed.com/users/rolo644u/code/ST7735/file/291ac9fb4d64/ST7735.cpp
                             // functions for drawing strings did not work when called; hastily made some functions to similar role (found at bottom of file)
#include "ST7565/ST7565.h"
ST7565* glcd;
//ST7735* screen;
//DigitalOut   RST_pin(D8);

#include "lvgl/lvgl.h" // from littlevGL: https://littlevgl.com/download
#define LVGL_TICK 5
#define TICKER_TIME 0.001*LVGL_TICK
void display_init(void); // function for initializing littleVGL
static void my_disp_flush_cb(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p); // function LittleVGL uses to send pixel values to screen
void main_screen(void); // function for drawing littleVGL objects to location on menu pages
Thread ticker_thread(osPriorityNormal, 8*1024, NULL, "ticker_thread");
static void ticker_task(void); // function for updating internal timing of LittleVGL
//static void btn_event_cb(lv_obj_t * btn, lv_event_t event);
void menu_function(int &screen_selected, int &menu_i, int &last_menu_i, int &max_index_val); // function for handling actions when "enter" button is pressed
bool button_read(lv_indev_drv_t * drv, lv_indev_data_t*data); // function for determining what to do when button is pressed
static void label_refresher_task(void * p); // function for handling dynamic labels for littleVGL
void button_drv_reg(void) // function for registering which function handles button presses in littleVGL
Thread screen_thread(osPriorityNormal, 8*1024, NULL, "screen_thread");
Mutex magMutex;

// updatable values to display on main screen
int global_magnitude = 0;               // current EMF value
int global_max_magnitude = 10;          // maximum EMF value of session
int global_avg_magnitude = 1520;        // average EMF value of session
int global_battery_val = 35;            // current battery value

// menu item variables with default values
bool storage_type_var = 0;
bool storage_en_var = 1;
int set_alarm_var = 20000; // when to trigger alarm (starts in mG)
int max_alarm = 30000;     // arbitrary max alarm value
int min_alarm = 2000;      // arbitrary min alarm value
bool alarm_en_var = 0;
bool rec_now_var = 0;      // 0 = stop (do not record), 1 = start (record if not paused)
bool rec_pause_var = 1;    // 0 = unpaused, 1 = paused (do not record)

// screen things
int menu_index = 0; // index for handling which menu item in the list is selected
lv_obj_t *label1;
lv_obj_t * scr;
lv_obj_t * menu1_scr;
lv_obj_t * menu2_scr;

// main menu things
lv_group_t * g;
lv_obj_t * button_array[4];
lv_obj_t * button_label_array[4];
lv_obj_t * current_EMF_val;
lv_obj_t * max_EMF_val;
lv_obj_t * battery_val;
lv_obj_t * avg_val;

// play/pause/stop, alarm, enable upload to lte, lte available
lv_obj_t * top_bar_array[4];
string top_bar_symbol[4] = {LV_SYMBOL_STOP, LV_SYMBOL_MUTE, LV_SYMBOL_UPLOAD, LV_SYMBOL_WIFI};

// menu 1 things
unsigned int global_name_index = 0; // which ID to display
string global_name_list[] = {"1 Alpha", "2 Beta", "3 Gamma", "4 Delta", "5 Epsilon", 
                            "6 Zeta", "7 Eta", "8 Theta", "9 Iota", "10 Kappa"}; // list of IDs

string menu1_text[] = {"ID:", "Storage:", "Stor EN:", "Alarm:", "Next"};
string label1_text[] = {global_name_list[0], "Internal", "Enabled", "20000", ""};
lv_obj_t * menu1_array[5];
lv_obj_t * label1_array[5];

// menu 2 things
string menu2_text[] = {"Back", "Units", "Field", "Fac RST", "Exit Menu"};
string label2_text[] = {"", "mG", "Magnetic", "", ""};
lv_obj_t * menu2_array[5];
lv_obj_t * label2_array[5];

// styles for what button looks like when it is or isn't selected
lv_style_t selected_btn_style;
lv_style_t default_btn_style;

// button things
InterruptIn right_button(A0); // button linked to XX pin on NUCELO board
InterruptIn left_button(A1);
InterruptIn enter_button(USER_BUTTON);
DigitalOut led(LED1);
DigitalOut led2(LED2);
bool button_flag = 0; // flags signifying whether a button was pressed
bool button2_flag = 0;
bool button3_flag = 0;
Timeout button_timeout;
Timeout button2_timeout;
Timeout button3_timeout;
volatile bool button_enabled = true;  // Used for debouncing
volatile bool button2_enabled = true; // Used for debouncing
volatile bool button3_enabled = true; // Used for debouncing
volatile int DEBOUNCE_TIME = 0.3;

// Enables button when bouncing is over
void button_enabled_cb(void)
{
    button_enabled = true;
    if (right_button == false)
    {
        button_flag = 1;
    }
}
// Enables button when bouncing is over
void button2_enabled_cb(void)
{
    button2_enabled = true;
    if (left_button == false)
    {
        button2_flag = 1;
    }
}
// Enables button when bouncing is over
void button3_enabled_cb(void)
{
    button3_enabled = true;
    if (enter_button == false)
    {
        button3_flag = 1;
    }
}

void released()
{
    led = 0;
    if (button_enabled) { // Disabled while the button is bouncing
        button_enabled = false;
        button2_timeout.attach(callback(button_enabled_cb), DEBOUNCE_TIME); // Debounce time
    }
}

void pressed(){ led = 1;}

void released2()
{
    led2 = 0;
    if (button2_enabled) { // Disabled while the button is bouncing
        button2_enabled = false;
        button2_timeout.attach(callback(button2_enabled_cb), DEBOUNCE_TIME); // Debounce time
    }
}

void pressed2(){led2 = 1;}

void released3()
{
    if (button3_enabled) { // Disabled while the button is bouncing
        button3_enabled = false;
        button3_timeout.attach(callback(button3_enabled_cb), DEBOUNCE_TIME); // Debounce time
    }
}

int main() {
    // currently have one of the attempted basic setups for using the ST7565
    glcd = new ST7565(D11,D13,D9,D8,D10);
    glcd->begin(0x18);
    glcd->display(); // show splashscreen
    wait_ms(2000);
    glcd->clear();
    glcd->setpixel(10, 10, BLACK); // display a pixel on screen
    glcd->display();        // show the changes to the buffer
    while(1) {}             // stop here for now


    // when/if above works, try basic setup for LittleVGL ** need to put code which sends LittleVGL information to screen (i.e. ST7565 driver library stuff) in "my_disp_flush_cb()"
	lv_init(); // init lvgl
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

    /*Create a Label on the currently active screen*/
	label1 = lv_label_create(lv_scr_act(), NULL);
    //current_EMF_val =  lv_label_create(scr, NULL);
    //lv_label_set_text(current_EMF_val, "Hello world!");
    //lv_obj_align(current_EMF_val, NULL, LV_ALIGN_CENTER, 0, 0);

	/*Modify the Label's text*/
	lv_label_set_text(label1, "Hello world!");

	/* Align the Label to the center
     * NULL means align on parent (which is the screen now)
     * 0, 0 at the end means an x, y offset after alignment*/
	lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, 0);

    ticker_thread.start(ticker_task); // starter ticker
    //lv_task_create(label_refresher_task, 100, LV_TASK_PRIO_MID, NULL);

    printf("made it to infinity loop\n");
	while (1) {}            // stop here to prevent termination of thread
}

// LittleVGL function that is called when LittleVGL wants to update the screen. Write to the screen using the display's driver library here
// "area" is the coordinates of what needs to be updated
// "color_p" is the color value of the pixels
void my_disp_flush_cb(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p)
{
    // depending on if screen for ST7565 works, put code to send that information to screen here (code for using ST7735 commented out in this function atm)

    /*
    // Get the outer boundary coordingates of the area to update
    uint16_t x = area->x1; 
    uint16_t y = area->y1;
    uint8_t hi, lo;

    // Tell the screen the rectangular area it needs to update
    screen->setAddrWindow(x, y, x+(area->x2-area->x1), y+(area->y2-area->y1));
    screen->dc->write(1);
    screen->ce->write(0);
 
    // write all the color pixels to the screen
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            //put_px(x, y, *color_p)
            hi = color_p->full >> 8;
		    lo = color_p->full;
		
            screen->spi->write(hi);
            screen->spi->write(lo);
            //screen->drawPixel( x, y, color_p->full);
            color_p++;
        }
    }
    screen->ce->write(1);
    */
    lv_disp_flush_ready(disp_drv);//IMPORTANT!!!* Inform the graphics library that you are ready with the flushing
}

// needed for internal timing of LittleVGL library 
static void ticker_task(void)
{
    while(1)
    {       
    lv_tick_inc(LVGL_TICK); 
    //Call lv_tick_inc(x) every x milliseconds in a Timer or Task (x should be between 1 and 10). 
    //It is required for the internal timing of LittlevGL.
    lv_task_handler(); 
    //Call lv_task_handler() periodically every few milliseconds. 
    //It will redraw the screen if required, handle input devices etc.
    ThisThread::sleep_for(LVGL_TICK);  //in msec
    }
}

// this task is called periodically. Put any dynamic values to display here. Have one case example for now.
static void label_refresher_task(void * p)
{
    // only update screen if previous values get changed
    static uint32_t prev_value_current_EMF = 0;
    static uint32_t prev_value_max_EMF = 0;
    static uint32_t prev_value_bat = 0;
    static uint32_t prev_value_avg_EMF = 0;
    //static char * prev_value_name = &global_name[0];
    static char buf[32];

    // update present EMF value label only if it is differnt from previous values (to avoid screen making redraws when not needed)
    if(prev_value_current_EMF != global_magnitude) {

        if(lv_obj_get_screen(current_EMF_val) == lv_scr_act()) {
            sprintf(buf, "%d", global_magnitude);
            lv_label_set_text(current_EMF_val, buf);
        }
        prev_value_current_EMF = global_max_magnitude;
    }

}






// below is setup for when ST7735 screen was used (starting from main() function) with more complete GUI menu setup
/*
Initialize LittleVGL with the function "display_init()", which sets up the relevant variables and links functions created in program to LittleVGL
    Also call "main_screen()" to arrange location of objects to screen within LittleVGL system
    Also call "button_drv_reg()" to link button handling function to LittleVGL (consider moving contents of function to "display_init()" instead)
        LittleVGL documentation website: https://docs.littlevgl.com/
    Initialize the thread "ticker_task()", which is used as timer to update internal timer of LittleVGL and request it make updates to screen
    Initialize the thread "label_refresher_task()" which is used to update label values for LittleVGL so it knows to redraw those elements on next screen update

Updating dynamic values:
    Update/change frequently changing values (e.g. current EMF value) in "label_refresher_task()"

Button presses:
    When button is pressed, button press is debounced, via "pressed()" and "button_enabled_cb()" (consider cleaning up how this is done) 
    "button_read()" is called, and sees which button was pressed. Handles navigation (left/right buttons) and calls "menu_function()" if enter button is pressed
    "menu_function" handles additional actions related to when the enter button is pressed while a menu item is selected

How things get updated in LittleVGL:
    - "ticker_task()" increments an internal timer in LittleVGL and calls a function to check for updates that need to be done
    - "label_refresher_task()" is also used to update label values for displaying frequently changing information 
    In set intervals, when LittleVGL decides to do a screen update, "my_disp_flush_cb()" gets called
        "my_disp_flush_cb()" gets the area of the screen that needs updating as a parameter, and sends that information to the screen
            i.e. it is the function where the user puts the code for the screen driver library
        "lv_disp_flush_ready()" within the above function tells LittleVGL it is done with sending information to the screen
    - Note: going through "label_refresher_task()" is for dealing with frequently updated/changed values. 
        Things drawn once (like in "main_screen()") or rarely updated (like in some parts of "menu_function()") do not go through that function
    - Note: when updating values, only re-assign values to the LittleVGL label variables when the value is different from its previous value
        (to avoid possibility of LittleVGL thinking that the value needs updating, and sending that information to the screen, even though that field is changed to the same value
        i.e. to avoid making unneeded updates to the screen)
*/
/*
int main(void) /////////////////////////////////////////////////////////////////////////////////////////////////////////
{
    // for verification that the program was successfully uploaded/executed
    printf("\r\n"); printf("     ****\r\n");printf("    **  **     \r\n");printf("   **    **    \r\n");printf("  ** ==== **   \r\n\n");printf("Identifier #46\n");// end of initial program verification

    // initialize ST7735 screen
    screen = new ST7735(D10, D9, D11, D13);
    RST_pin = 0; wait_ms(50);
    RST_pin = 1; wait_ms(50);
    screen->initR(INITR_GREENTAB);
    screen->setRotation(0);wait_ms(100);
    screen->fillScreen(ST7735_BLACK); // make screen black

    // set up button debouncing (make cleaner later)
    right_button.rise(&pressed);
    right_button.fall(&released);
    left_button.rise(&pressed2);
    left_button.fall(&released2);
    enter_button.fall(&released3);

    // things that need to be done to set up LittleVGL
    display_init();                                                     // initialize LittleVGL
    main_screen();                                                      // draw all objects to LittleVGL menu pages
    ticker_thread.start(ticker_task);                                   // start LittleVGL's internal timer thread
    lv_task_create(label_refresher_task, 100, LV_TASK_PRIO_MID, NULL);  // start thread for changing LittleVGL label values (updating dynamic display values)
    button_drv_reg();                                                   // for registering which function handles button presses


    // other actions for EMF Monitor would go here


    printf("made it to infinite loop\n"); while(true){ }                // prevent thread from terminating
    return 0;
}

// this function is called periodically and is where any dynamic LittleVGL labels are updated
//      *found that the only way to change a displayed value in LittleVGL is to re-assign a value to a label then draw that label again
static void label_refresher_task(void * p)
{
    // initialize local variables. Used to capture snapshot of these values during each update to keep data synchronized
    static int local_magnitude;
    static int local_max_magnitude;
    static int local_avg_magnitude;
    static int local_battery_val;
    // local variables for holding previous values, so LittleVGL labels (and the screen by extension) are only re-drawn on the screen when the value changes
    static uint32_t prev_value_current_EMF = 0;
    static uint32_t prev_value_max_EMF = 0;
    static uint32_t prev_value_bat = 0;
    static uint32_t prev_value_avg_EMF = 0;
    static char buf[32];

    // make local copy of variables to synchronize data snapshot
    magMutex.lock();
    local_magnitude = global_magnitude;
    local_max_magnitude = global_max_magnitude;
    local_battery_val = global_battery_val;
    local_avg_magnitude = global_avg_magnitude;
    //local_name = &global_name[0];
    magMutex.unlock();

    // update present EMF value
    if(prev_value_current_EMF != global_magnitude) {

        if(lv_obj_get_screen(current_EMF_val) == lv_scr_act()) {
            sprintf(buf, "%d", global_magnitude);
            lv_label_set_text(current_EMF_val, buf);
        }
        prev_value_current_EMF = global_max_magnitude;
    }
    
    // update max EMF value
    if(prev_value_max_EMF != global_max_magnitude) {

        if(lv_obj_get_screen(max_EMF_val) == lv_scr_act()) {
            sprintf(buf, "%d", global_max_magnitude);
            lv_label_set_text(max_EMF_val, buf);
        }
        prev_value_max_EMF = global_max_magnitude;
    }

    // update avg EMF value
    if(prev_value_avg_EMF != global_avg_magnitude) {

        if(lv_obj_get_screen(avg_val) == lv_scr_act()) {
            sprintf(buf, "%d", global_avg_magnitude);
            lv_label_set_text(avg_val, buf);
        }
        prev_value_avg_EMF = global_avg_magnitude;
    }

    // update battery % value
    if(prev_value_bat != global_battery_val) {

        if(lv_obj_get_screen(battery_val) == lv_scr_act()) {
            sprintf(buf, "%d", global_battery_val);
            lv_label_set_text(battery_val, buf);
        }
        prev_value_bat = global_battery_val;
    }
}

// LittleVGL related function: for setting up styles and drawing all objects to the littleVGL displays (in this context, the displays refer to the main, setting 1, and setting 2 pages of the menu)
// call after LittleVGL is initialized
void main_screen(void)
{
    char buffer[20]; // character array to act as buffer for text

    menu1_scr=lv_obj_create(NULL, NULL);
    menu2_scr=lv_obj_create(NULL, NULL);
    scr=lv_obj_create(NULL, NULL);

	static lv_style_t style_new;                         //Styles can't be local variables
    lv_style_copy(&style_new, &lv_style_pretty);         //Copy a built-in style as a starting point
	style_new.text.font = &lv_font_roboto_12;

	static lv_style_t style_big_font;
    lv_style_copy(&style_big_font, &lv_style_pretty);
    style_big_font.text.font = &lv_font_roboto_22;

    lv_style_copy(&selected_btn_style, &lv_style_pretty);
    lv_style_copy(&default_btn_style, &lv_style_plain);

	lv_obj_t * label;
    lv_obj_t * current_EMF_label;
    lv_obj_t * max_EMF_label;
    lv_obj_t * battery_label;
    lv_obj_t * avg_label;

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

    avg_val =  lv_label_create(scr, NULL);
    lv_obj_set_style(avg_val, &style_new); 
    avg_label =  lv_label_create(scr, NULL); 
    lv_obj_set_style(avg_label, &style_new);

    // place location of main menu text items:
    // current EMF value
    lv_label_set_text(current_EMF_label, "                mG");  
    lv_obj_set_y(current_EMF_label, 10);
    lv_obj_set_x(current_EMF_label, 1);

    lv_label_set_text(current_EMF_val, "00000");
	lv_obj_align(current_EMF_val, current_EMF_label, LV_ALIGN_OUT_BOTTOM_RIGHT, -35, -26);
    lv_label_set_align(current_EMF_val,LV_LABEL_ALIGN_RIGHT);

    // maximum EMF value
    lv_label_set_text(max_EMF_label, "Max:               mG");  
    lv_obj_set_y(max_EMF_label, 29);
    lv_obj_set_x(max_EMF_label, 1);
    lv_label_set_text(max_EMF_val, "00000");
	lv_obj_align(max_EMF_val, max_EMF_label, LV_ALIGN_OUT_BOTTOM_RIGHT, -20, -14);
    lv_label_set_align(max_EMF_val,LV_LABEL_ALIGN_RIGHT);

    // average EMF value
    lv_label_set_text(avg_label, " Avg:               mG");  
    lv_obj_set_y(avg_label, 39);
    lv_obj_set_x(avg_label, 1);
    lv_label_set_text(avg_val, "00000");
	lv_obj_align(avg_val, avg_label, LV_ALIGN_OUT_BOTTOM_RIGHT, -20, -14);
    lv_label_set_align(avg_val,LV_LABEL_ALIGN_RIGHT);

    // Battery
    lv_label_set_text(battery_label, LV_SYMBOL_BATTERY_3 "        %" );  
    lv_obj_set_y(battery_label, 1);
    lv_obj_set_x(battery_label, 1);
    lv_label_set_text(battery_val, "100");
	lv_obj_align(battery_val, battery_label, LV_ALIGN_OUT_RIGHT_MID, -32, 0);

    // buttons ////////////////////////////////////////////////////////////////////////
    // 0 = menu, 1 = start/pause, 2 = stop, 3 = alarm mute
    button_array[0] = lv_btn_create(scr, NULL);       
    //lv_obj_set_event_cb(button_array[0], btn_event_cb);                         //Set function to be called when the button is released
    lv_obj_set_style(button_array[0], &selected_btn_style);
    
    button_label_array[0] = lv_label_create(button_array[0], NULL);
    lv_obj_set_style(button_label_array[0], &style_new);
    lv_label_set_text(button_label_array[0], LV_SYMBOL_SETTINGS);
    //lv_btn_set_toggle(menu_btn, true);
    lv_obj_set_y(button_array[0], 51);
    lv_obj_set_x(button_array[0], 1);
    lv_obj_set_size(button_array[0], 20,16);
    lv_group_add_obj(g, button_array[0]);

    button_array[1] = lv_btn_create(scr, NULL);          
    //lv_obj_set_event_cb(button_array[1], btn_event_cb);                         //Set function to be called when the button is released
    lv_obj_set_style(button_array[1], &default_btn_style);
    button_label_array[1] = lv_label_create(button_array[1], NULL);
    lv_obj_set_style(button_label_array[1], &style_new);
    lv_label_set_text(button_label_array[1], LV_SYMBOL_PLAY);
    lv_obj_set_size(button_array[1], 20,16);
    lv_obj_align(button_array[1], button_array[0], LV_ALIGN_OUT_RIGHT_MID, 1, 0);
    lv_group_add_obj(g, button_array[1]);

    button_array[2] = lv_btn_create(scr, NULL);         
    //lv_obj_set_event_cb(button_array[2], btn_event_cb);                         //Set function to be called when the button is released
    lv_obj_set_style(button_array[2], &default_btn_style);
    button_label_array[2] = lv_label_create(button_array[2], NULL);
    lv_obj_set_style(button_label_array[2] , &style_new);
    lv_label_set_text(button_label_array[2] , LV_SYMBOL_STOP);
    lv_obj_set_size(button_array[2], 20,16);
    lv_obj_align(button_array[2], button_array[1], LV_ALIGN_OUT_RIGHT_MID, 1, 0);
    lv_group_add_obj(g, button_array[2]);

    button_array[3] = lv_btn_create(scr, NULL);       
    //lv_obj_set_event_cb(button_array[3], btn_event_cb);                         //Set function to be called when the button is released
    lv_obj_set_style(button_array[3], &default_btn_style);
    button_label_array[3]  = lv_label_create(button_array[3], NULL);
    lv_obj_set_style(button_label_array[3], &style_new);
    lv_label_set_text(button_label_array[3], LV_SYMBOL_VOLUME_MAX);
    lv_obj_set_size(button_array[3], 20,16);
    lv_obj_align(button_array[3], button_array[2], LV_ALIGN_OUT_RIGHT_MID, 1, 0);
    lv_group_add_obj(g, button_array[3]);

    lv_group_focus_obj(button_array[0]);

    // 0 = play/pause/stop, 1 = alarm, 2 = enable upload to lte, 3 = lte available
    for (int i = 0; i < 4; i++)
    {
        top_bar_array[i] = lv_label_create(scr, NULL);  
        lv_obj_set_style(top_bar_array[i], &style_new); 
        lv_label_set_text(top_bar_array[i], strncpy(buffer, top_bar_symbol[i].c_str(), sizeof(buffer)));
        lv_obj_set_size(top_bar_array[i], 16, 16);
        lv_obj_set_y(top_bar_array[i], 1);
        lv_obj_set_x(top_bar_array[i], 60+i*16);
    } 

    // 0 = ID, 1 = storage type, 2 = storage enable, 3 = alarm level, 4 = next
    for (int i = 0; i < 5; i++)
    {
        menu1_array[i] = lv_btn_create(menu1_scr, NULL);          
        //lv_obj_set_event_cb(menu1_array[i], btn_event_cb);
        lv_obj_set_style(menu1_array[i], &default_btn_style);

        lv_obj_set_size(menu1_array[i], 55, 13);
        lv_obj_set_y(menu1_array[i], 1+ i*13);
        lv_obj_set_x(menu1_array[i], 1);

        label = lv_label_create(menu1_array[i], NULL);
        lv_obj_set_style(label, &style_new);
        lv_label_set_text(label, strncpy(buffer, menu1_text[i].c_str(), sizeof(buffer)));
        lv_label_set_align(label,LV_LABEL_ALIGN_LEFT);

        label1_array[i] =  lv_label_create(menu1_scr, NULL); 
        lv_obj_set_style(label1_array[i], &style_new);
        lv_label_set_text(label1_array[i], strncpy(buffer, label1_text[i].c_str(), sizeof(buffer)));  
        lv_obj_align(label1_array[i],  menu1_array[i], LV_ALIGN_OUT_RIGHT_MID, 1, 0);
    } 

    // 0 = back, 1 = change units, 2 = change field, 3 = factory reset, 4 = exit menu
    for (int i = 0; i < 5; i++)
    {
        menu2_array[i] = lv_btn_create(menu2_scr, NULL);          
        //lv_obj_set_event_cb(menu2_array[i], btn_event_cb);
        lv_obj_set_style(menu2_array[i], &default_btn_style);

        lv_obj_set_size(menu2_array[i], 55, 13);
        lv_obj_set_y(menu2_array[i], 1+ i*13);
        lv_obj_set_x(menu2_array[i], 1);

        label = lv_label_create(menu2_array[i], NULL);
        lv_obj_set_style(label, &style_new);
        lv_label_set_text(label, strncpy(buffer, menu2_text[i].c_str(), sizeof(buffer)));
        lv_label_set_align(label,LV_LABEL_ALIGN_LEFT);

        label2_array[i] =  lv_label_create(menu2_scr, NULL); 
        lv_obj_set_style(label2_array[i], &style_new);
        lv_label_set_text(label2_array[i], strncpy(buffer, label2_text[i].c_str(), sizeof(buffer)));  
        lv_obj_align(label2_array[i],  menu2_array[i], LV_ALIGN_OUT_RIGHT_MID, 1, 0);
    } 

    // cutoff for new screen... just for screen sizing purposes. Can remove whenever they're not needed anymore
    static lv_point_t line_points[] = { {1, 65}, {123, 65}};static lv_style_t style_line;
    lv_style_copy(&style_line, &lv_style_plain);style_line.line.width = 1;
    lv_obj_t * line1;line1 = lv_line_create(scr, NULL);
    lv_line_set_points(line1, line_points, 2);lv_line_set_style(line1, LV_LINE_STYLE_MAIN, &style_line);
    lv_obj_t * line2;line2 = lv_line_create(menu1_scr, NULL);
    lv_line_set_points(line1, line_points, 2);lv_line_set_style(line2, LV_LINE_STYLE_MAIN, &style_line);
    lv_obj_t * line3;line3 = lv_line_create(menu2_scr, NULL);
    lv_line_set_points(line3, line_points, 2);lv_line_set_style(line3, LV_LINE_STYLE_MAIN, &style_line);

    lv_scr_load(scr);
}

// LittleVGL function: called when button press is detected. Put code to determine which button was pressed and what to do about that button press here
// calls "menu_function" to handle case where "enter" button is pressed
bool button_read(lv_indev_drv_t * drv, lv_indev_data_t*data){  
    static int max_index = sizeof(button_array)/sizeof(button_array[0])-1;
    static int last_index = 0;
    static int current_screen = 0;

    // go right
    if (button_flag == true)
    {
        button_flag = false;
        menu_index++;
        if(menu_index > max_index)
        {
            menu_index = 0;
        }
    }

    // go left
    if (button2_flag == true)
    {
        button2_flag = false;
        menu_index--;
        if(menu_index < 0)
        {
            menu_index = max_index;
        }
    }

    // enter
    if (button3_flag == true)
    {
        button3_flag = false;
        menu_function(current_screen, menu_index, last_index, max_index); // do different operation based on which button is pressed
    }

    // whenever a different menu item is selected, un-highlight the previously selected button/menu item and highlight the currently selected button/menu item
    if (last_index != menu_index)
    {
        switch(current_screen)
        {
            case 0: // when in main screen
            printf("screen: %d, menu index: %d, last index: %d\n", current_screen, menu_index, last_index); // for debugging
                lv_obj_set_style(button_array[menu_index], &selected_btn_style); // change style of selected button to "selected_btn_style" (so it stands out)
                lv_obj_set_style(button_array[last_index], &default_btn_style);  // change style of prior selected button to "default_btn_style" (un-highlight)
            break;
            case 1: // when in menu screen 1
            printf("screen: %d, menu index: %d, last index: %d\n", current_screen, menu_index, last_index); // for debugging
                lv_obj_set_style(menu1_array[menu_index], &selected_btn_style);
                lv_obj_set_style(menu1_array[last_index], &default_btn_style);
            break;
            case 2: // when in menu screen 2
            printf("screen: %d, menu index: %d, last index: %d\n", current_screen, menu_index, last_index); // for debugging
                lv_obj_set_style(menu2_array[menu_index], &selected_btn_style);
                lv_obj_set_style(menu2_array[last_index], &default_btn_style);
            break;
            default:
            break;
        }
        
        last_index = menu_index;
    }
    return false;                    //No buffering now so no more data read
}

// Called from "button_read" function when "enter" button is pressed
// Do a certain action depending on which screen and which menu item is selected
void menu_function(int &screen_selected, int &menu_i, int &last_menu_i, int &max_index_val)
{
    static unsigned int name_list_index = 0;
    static char buffer[32];


    // static variables defined here to avoid hard-coding values (move to .h file as constants later)
    // index values for where top bar objects are (to right of battery %)
    static int tb_pos_0 = 0;
    static int tb_pos_1 = 1;
    static int tb_pos_2 = 2;

    // index values for menu items (zero indexed to match index numbering)
    static int menu_pos_0 = 0;
    static int menu_pos_1 = 1;
    static int menu_pos_2 = 2;
    static int menu_pos_3 = 3;
    static int menu_pos_4 = 4;

    // index values for where bottom bar buttons are
    static int setting_btn = 0;
    static int start_rec_btn = 1;
    static int stop_rec_btn = 2;
    static int mute_btn = 3;

    // index values for main screen and setting screens
    static int main_scn = 0;
    static int setting_scn_1 = 1;
    static int setting_scn_2 = 2;

    // other values
    static int alarm_thresh_inc = 500; // for now, have threshold incremented in units of XXX

    // do different action based on screen selected. Outer switch statement handles which menu is selected, inner switch statements handle menu for that screen
    switch(screen_selected)
    {
        case 0: // main screen (displays EMF values)
            switch(menu_i)
            {
                case : // settings button
                    lv_obj_set_style(button_array[menu_i], &default_btn_style);
                    lv_obj_set_style(menu1_array[0], &selected_btn_style);
                    menu_i = 0;          // reset index of menu
                    last_menu_i=0;
                    screen_selected = setting_scn_1; // go to next screen
                    max_index_val = sizeof(menu1_array)/sizeof(menu1_array[0])-1; // change max array index
                    lv_scr_load(menu1_scr);
                break;

                case 1: // start/pause recording button
                    // update flags
                    rec_now_var = 1; 
                    rec_pause_var = !rec_pause_var;
                    
                    // change top bar and lower bar icons
                    if (rec_pause_var)
                    {
                        lv_label_set_text(top_bar_array[tb_pos_0], LV_SYMBOL_PAUSE);  
                        lv_label_set_text(button_label_array[start_rec_btn], LV_SYMBOL_PLAY);
                    }
                    else
                    {
                        lv_label_set_text(top_bar_array[tb_pos_0], LV_SYMBOL_PLAY); 
                        lv_label_set_text(button_label_array[start_rec_btn], LV_SYMBOL_PAUSE);  
                    }
                break;

                case 2: // stop recording button
                    // update flags                
                    rec_now_var = 0;
                    rec_pause_var = 1;

                    // change top bar and lower bar icons
                    lv_label_set_text(top_bar_array[tb_pos_0], LV_SYMBOL_STOP);
                    lv_label_set_text(button_label_array[start_rec_btn], LV_SYMBOL_PLAY); 
                break;

                case 3: // mute alarm button
                    alarm_en_var = !alarm_en_var; // update flag

                    // change top bar and lower bar icons 
                    if (alarm_en_var)
                    {
                        lv_label_set_text(top_bar_array[tb_pos_1], LV_SYMBOL_VOLUME_MAX);
                        lv_label_set_text(button_label_array[mute_btn], LV_SYMBOL_MUTE);
                    }
                    else
                    {
                        lv_label_set_text(top_bar_array[tb_pos_1], LV_SYMBOL_MUTE);
                        lv_label_set_text(button_label_array[mute_btn], LV_SYMBOL_VOLUME_MAX);
                    }
                break;

                default: // OOB menu index passed to this function
                break;
                
            }
        break;

        case 1: // menu 1 screen
            switch(menu_i)
            {
                case 0: // change ID
                    name_list_index ++; // change index of which name to display (doubles as flag too)
                    // change which name to display on menu
                    label1_text[menu_pos_0] = global_name_list[name_list_index%(sizeof(global_name_list)/sizeof(global_name_list[0])-1)];
                    lv_label_set_text(label1_array[menu_pos_0], strncpy(buffer, label1_text[0].c_str(), sizeof(buffer)));  
                break;

                case 1: // storage type
                    storage_type_var = !storage_type_var; // update flag
                    if (storage_type_var)
                    {
                        lv_label_set_text(label1_array[menu_pos_1], "Internal");   // change menu text
                        lv_label_set_text(top_bar_array[tb_pos_2], LV_SYMBOL_SAVE);// change top bar icon (on main screen)
                    }
                    else
                    {
                        lv_label_set_text(label1_array[menu_pos_1], "LTE");         // change menu text
                        lv_label_set_text(top_bar_array[tb_pos_2], LV_SYMBOL_UPLOAD); // change top bar icon (on main screen)
                    } 
                break;

                // * this feature may not be needed (re-purpose?)
                case 2: // enable lte storage
                    storage_en_var = !storage_en_var; // update flag
                    if (storage_en_var)
                    {
                        lv_label_set_text(label1_array[menu_pos_2], "Enabled"); // change menu text           
                    }
                    else
                    {
                        lv_label_set_text(label1_array[menu_pos_2], "Disabled"); // change menu text
                    } 
                break;

                case 3: // set alarm level
                    set_alarm_var += alarm_thresh_inc; // change alarm threshold value

                    if (set_alarm_var > max_alarm)     // rollover alarm threshold (*change how this is done if 4th input button or if keypad entering style of this is implemented)
                        set_alarm_var = min_alarm;

                    sprintf(buffer, "%d", set_alarm_var);
                    lv_label_set_text(label1_array[menu_pos_3], buffer); // update menu text
                break;

                case 4: // go to next menu
                    lv_obj_set_style(menu1_array[menu_i], &default_btn_style);// reset which menu item to select
                    lv_obj_set_style(menu2_array[0], &selected_btn_style);
                    menu_i = 0;          // reset index of menu
                    last_menu_i=0;
                    screen_selected = setting_scn_2; // go to next screen
                    lv_scr_load(menu2_scr);
                    max_index_val = sizeof(menu2_array)/sizeof(menu2_array[0])-1; // change max array index
                break;

                default: // OOB menu index passed to this function
                break;
            }
        break;

        case 2: // menu 2 screen
            switch(menu_i)
            {
                case 0: // back to previous menu
                    lv_obj_set_style(menu2_array[menu_i], &default_btn_style); // reset which button is selected
                    lv_obj_set_style(menu1_array[0], &selected_btn_style);
                    menu_i = 0;          // reset index of menu
                    last_menu_i=0;
                    screen_selected = setting_scn_1; // go to prior screen
                    lv_scr_load(menu1_scr);
                    max_index_val = sizeof(menu1_array)/sizeof(menu1_array[0])-1; // change max array index
                break;

                case 1: // change units (WIP)
                break;

                case 2: // change field (WIP)
                break;

                case 3: // factory reset (WIP)
                break;

                case 4: // go to next menu
                    lv_obj_set_style(menu2_array[menu_i], &default_btn_style); // reset which button is selected
                    lv_obj_set_style(button_array[0], &selected_btn_style);
                    menu_i = 0;          // reset index of menu
                    last_menu_i=0;
                    screen_selected = main_scn; // go to next screen
                    lv_scr_load(scr);
                    max_index_val = sizeof(button_array)/sizeof(button_array[0])-1; // change max array index

                default: // OOB menu index passed to this function
                break;
                
            }
        break;
        default: // for some reason, unknown screen passed to this function
        break;
    }
    return;
}

// LittleVGL function: called when LittleVGL wants to update the screen. Write to the screen using the display's driver library here
// "area" is the coordinates of what needs to be updated
// "color_p" is the color value of the pixels
void my_disp_flush_cb(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p)
{
    // get the outer boundary coordinates of the area to update
    uint16_t x = area->x1; 
    uint16_t y = area->y1;
    uint8_t hi, lo;

    // tell the screen the rectangular area it needs to update
    screen->setAddrWindow(x, y, x+(area->x2-area->x1), y+(area->y2-area->y1));
    screen->dc->write(1);
    screen->ce->write(0);
 
    // write all the color pixels to the screen
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            hi = color_p->full >> 8;
		    lo = color_p->full;		
            screen->spi->write(hi);
            screen->spi->write(lo);
            color_p++;
        }
    }
    screen->ce->write(1);
    //IMPORTANT!!!* Inform the graphics library that you are ready with the flushing
    lv_disp_flush_ready(disp_drv);
}

// LittleVGL related function: for initializing LittleVGL and linking functions to it
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

// LittleVGL related thread: increment LittleVGL's internal timer periodically with # of milliseconds since last update (so library knows to make updates)
static void ticker_task(void)
{
    while(1)
    {       
        lv_tick_inc(LVGL_TICK); // increment LittleVGL's internal timer with lv_tick_inc(x), where x is the milliseconds since the last update (x should be b/w 1 and 10)
        lv_task_handler();      // tell LittleVGL to redraw information to the screen if needed, handle input devices, etc.
        ThisThread::sleep_for(LVGL_TICK);  //in msec
    }
}

// registers function for handling button presses
void button_drv_reg(void)
{
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;//LV_INDEV_TYPE_BUTTON;               
    indev_drv.read_cb = button_read;   
    g = lv_group_create();

    static lv_indev_t * my_indev = lv_indev_drv_register(&indev_drv);//Register the driver in LittlevGL and save the created input device object
    lv_indev_set_group(my_indev, g);
}




// Misc un-used functions archives
*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Below are functions quickly made to display strings on the ST7735 before LittleVGL worked

/*
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
        }else if ((string_array[index] == '.'))
        {
            screen->drawOneChar(SPACING + col*(SPACING + CHAR_COL),  SPACING + line*(SPACING + CHAR_ROW), &period[0][0]);
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
*/




// could not get working properly; not needed?
/*

 // Called when a button is released
 // @param btn pointer to the released button
 // @param event the triggering event
 // @return LV_RES_OK because the object is not deleted in this function
static void btn_event_cb(lv_obj_t * btn, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        //Increase the button width
        //lv_coord_t width = lv_obj_get_width(btn);
        //lv_obj_set_width(btn, width + 20);

        
        global_battery_val++;
        printf("Clicked\n");
    }
    else if (event == LV_EVENT_VALUE_CHANGED)
    {
        printf("Toggled\n");
    }
}
*/

