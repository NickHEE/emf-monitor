// screen_char_things.h
// This file has bitmaps for displaying characters to the ST7735 screen. Functions to make use of these bitmaps are found
//	near the bottom of the AvnetBG96_azure_client.cpp file within this branch.
// Since LittleVGL handles characters/strings already, using this file is unneeded.

#pragma once
#define ROW 10 				// rows of display
#define COL 50 				// columns of display

short led_display[ROW][COL]; // array holding all the "pixels" of the OLED screen


#define INPUT_LENGTH 6
#define DUMMY_VAL 10

#define IMAGE_HEIGHT 10
#define IMAGE_WIDTH 50
#define CHAR_ROW 8         // character height (# of rows)
#define CHAR_COL 5         // character width (# of col)
#define SPACING 1          // spacing between charcters

#define BACKGROUND 0x0000	// background color of led_display
#define TEXT 0x0fff			// 0xffff text color of led_driver

                             // shorthand notation for creating character "bit" masks
#define B BACKGROUND
#define T TEXT

const short one[CHAR_ROW][CHAR_COL] = {
      { T, T, T, B, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { T, T, T, T, T },
   };
const short two[CHAR_ROW][CHAR_COL] = {
      { B, T, T, T, B },
      { T, B, B, B, T },
      { B, B, B, B, T },
      { B, B, B, T, B },
      { B, B, T, B, B },
      { B, T, B, B, B },
      { T, B, B, B, B },
      { T, T, T, T, T },
   };
const short three[CHAR_ROW][CHAR_COL] = {
      { T, T, T, T, T },
      { B, B, B, T, B },
      { B, B, T, B, B },
      { B, T, T, T, B },
      { B, B, B, B, T },
      { B, B, B, B, T },
      { T, B, B, B, T },
      { B, T, T, T, B },
   };

const short four[CHAR_ROW][CHAR_COL] = {
      { T, B, B, T, B },
      { T, B, B, T, B },
      { T, B, B, T, B },
      { T, B, B, T, B },
      { T, T, T, T, T },
      { B, B, B, T, B },
      { B, B, B, T, B },
      { B, B, B, T, B },
   };
const short five[CHAR_ROW][CHAR_COL] = {
      { T, T, T, T, T },
      { T, B, B, B, B },
      { T, B, B, B, B },
      { T, T, T, T, B },
      { B, B, B, B, T },
      { B, B, B, B, T },
      { T, B, B, B, T },
      { B, T, T, T, B },
   };
const short six[CHAR_ROW][CHAR_COL] = {
      { B, T, T, T, B },
      { T, B, B, B, T },
      { T, B, B, B, B },
      { T, T, T, T, B },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { B, T, T, T, B },
   };
const short seven[CHAR_ROW][CHAR_COL] = {
      { T, T, T, T, T },
      { B, B, B, B, T },
      { B, B, B, T, B },
      { B, B, B, T, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { B, T, B, B, B },
      { B, T, B, B, B },
   };
const short eight[CHAR_ROW][CHAR_COL] = {
      { B, T, T, T, B },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { B, T, T, T, B },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { B, T, T, T, B },
   };
const short nine[CHAR_ROW][CHAR_COL] = {
      { B, T, T, T, B },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { B, T, T, T, T },
      { B, B, B, B, T },
      { B, B, B, B, T },
      { B, B, B, B, T },
      { B, B, B, B, T },
   };

const short zero[CHAR_ROW][CHAR_COL] = {
      { B, T, T, T, B },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { B, T, T, T, B },
   };
const short percent[CHAR_ROW][CHAR_COL] = {
      { B, B, B, B, B },
      { B, B, B, B, B },
      { T, B, B, B, T },
      { B, B, B, T, B },
      { B, B, T, B, B },
      { B, T, B, B, B },
      { T, B, B, B, T },
      { B, B, B, B, B },
   };
const short space[CHAR_ROW][CHAR_COL] = {
      { B, B, B, B, B },
      { B, B, B, B, B },
      { B, B, B, B, B },
      { B, B, B, B, B },
      { B, B, B, B, B },
      { B, B, B, B, B },
      { B, B, B, B, B },
      { B, B, B, B, B },
   };
   const short colon[CHAR_ROW][CHAR_COL] = {
      { B, B, B, B, B },
      { B, B, B, B, B },
      { B, B, T, B, B },
      { B, B, B, B, B },
      { B, B, B, B, B },
      { B, B, T, B, B },
      { B, B, B, B, B },
      { B, B, B, B, B },
   };
const short m[CHAR_ROW][CHAR_COL] = {
      { B, B, B, B, B },
      { B, B, B, B, B },
      { B, B, B, B, B },
      { B, B, B, B, B },
      { B, T, B, T, B },
      { T, B, T, B, T },
      { T, B, T, B, T },
      { T, B, T, B, T },
   };
const short A_[CHAR_ROW][CHAR_COL] = {
      { B, T, T, T, B },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, T, T, T, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
   };
   const short B_[CHAR_ROW][CHAR_COL] = {
      { T, T, T, T, B },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, T, T, T, B },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, T, T, T, B },
   };
   const short C_[CHAR_ROW][CHAR_COL] = {
      { B, T, T, T, B },
      { T, B, B, B, T },
      { T, B, B, B, B },
      { T, B, B, B, B },
      { T, B, B, B, B },
      { T, B, B, B, B },
      { T, B, B, B, T },
      { B, T, T, T, B },
   };
   const short D_[CHAR_ROW][CHAR_COL] = {
      { T, T, T, B, B },
      { T, B, B, T, B },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, T, B },
      { T, T, T, B, B },
   };
   const short E_[CHAR_ROW][CHAR_COL] = {
      { T, T, T, T, T },
      { T, B, B, B, B },
      { T, B, B, B, B },
      { T, T, T, B, B },
      { T, B, B, B, B },
      { T, B, B, B, B },
      { T, B, B, B, B },
      { T, T, T, T, T },
   };
   const short F_[CHAR_ROW][CHAR_COL] = {
      { T, T, T, T, T },
      { T, B, B, B, B },
      { T, B, B, B, B },
      { T, T, T, B, B },
      { T, B, B, B, B },
      { T, B, B, B, B },
      { T, B, B, B, B },
      { T, B, B, B, B },
   };
	const short G_[CHAR_ROW][CHAR_COL] = {
      { B, T, T, T, B },
      { T, B, B, B, T },
      { T, B, B, B, B },
      { T, B, B, B, B },
      { T, B, B, T, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { B, T, T, T, B },
   };
   const short H_[CHAR_ROW][CHAR_COL] = {
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, T, T, T, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
   };
   const short I_[CHAR_ROW][CHAR_COL] = {
      { T, T, T, T, T },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { T, T, T, T, T },
   };
   const short J_[CHAR_ROW][CHAR_COL] = {
      { B, B, B, T, T },
      { B, B, B, B, T },
      { B, B, B, B, T },
      { B, B, B, B, T },
      { B, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { B, T, T, T, B },
   };
   const short K_[CHAR_ROW][CHAR_COL] = {
      { T, B, B, B, T },
      { T, B, B, T, B },
      { T, B, T, B, B },
      { T, T, B, B, B },
      { T, T, B, B, B },
      { T, B, T, B, B },
      { T, B, B, T, B },
      { T, B, B, B, T },
   };
   const short L_[CHAR_ROW][CHAR_COL] = {
      { T, B, B, B, B },
      { T, B, B, B, B },
      { T, B, B, B, B },
      { T, B, B, B, B },
      { T, B, B, B, B },
      { T, B, B, B, B },
      { T, B, B, B, B },
      { T, T, T, T, T },
   };
   const short M_[CHAR_ROW][CHAR_COL] = {
      { T, B, B, B, T },
      { T, T, B, T, T },
      { T, B, T, B, T },
      { T, B, T, B, T },
      { T, B, T, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
   };
   const short N_[CHAR_ROW][CHAR_COL] = {
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, T, B, B, T },
      { T, B, T, B, T },
      { T, B, B, T, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
   };
   const short O_[CHAR_ROW][CHAR_COL] = {
      { B, T, T, T, B },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { B, T, T, T, B },
   };
   const short P_[CHAR_ROW][CHAR_COL] = {
      { B, T, T, T, B },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, T, T, T, B },
      { T, B, B, B, B },
      { T, B, B, B, B },
      { T, B, B, B, B },
   };
   const short Q_[CHAR_ROW][CHAR_COL] = {
      { B, T, T, T, B },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, T, B, T },
      { T, B, B, T, T },
      { B, T, T, T, T },
   };
   const short R_[CHAR_ROW][CHAR_COL] = {
      { T, T, T, T, B },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, T, T, T, B },
      { T, B, T, B, B },
      { T, B, B, T, B },
      { T, B, B, B, T },
   };
   const short S_[CHAR_ROW][CHAR_COL] = {
      { B, T, T, T, T },
      { T, B, B, B, B },
      { T, B, B, B, B },
      { B, T, T, T, B },
      { B, B, B, B, T },
      { B, B, B, B, T },
      { B, B, B, B, T },
      { T, T, T, T, B },
   };
   const short T_[CHAR_ROW][CHAR_COL] = {
      { T, T, T, T, T },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
   };
   const short U_[CHAR_ROW][CHAR_COL] = {
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { B, T, T, T, B },
   };
   const short V_[CHAR_ROW][CHAR_COL] = {
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { B, T, B, T, B },
      { B, T, B, T, B },
      { B, T, B, T, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
   };
   const short W_[CHAR_ROW][CHAR_COL] = {
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, B, B, T },
      { T, B, T, B, T },
      { T, B, T, B, T },
      { T, B, T, B, T },
      { B, T, B, T, B },
   };
   const short X_[CHAR_ROW][CHAR_COL] = {
      { T, B, B, B, T },
      { T, B, B, B, T },
      { B, T, B, T, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { B, T, B, T, B },
      { T, B, B, B, T },
      { T, B, B, B, T },
   };
   const short Y_[CHAR_ROW][CHAR_COL] = {
      { T, B, B, B, T },
      { T, B, B, B, T },
      { B, T, B, T, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
   };
   const short Z_[CHAR_ROW][CHAR_COL] = {
      { T, T, T, T, T },
      { B, B, B, B, T },
      { B, B, B, T, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { B, T, B, B, B },
      { T, B, B, B, B },
      { T, T, T, T, T },
   };
const short LTE_3bar[CHAR_ROW][CHAR_COL] = {
      { B, B, B, B, T },
      { B, B, B, B, T },
      { B, B, T, B, T },
      { B, B, T, B, T },
      { T, B, T, B, T },
      { T, B, T, B, T },      
      { B, B, B, B, B },
      { T, T, T, T, T },
   };
   const short LTE_2bar[CHAR_ROW][CHAR_COL] = {
      { B, B, B, B, B },
      { B, B, B, B, B },
      { B, B, T, B, B },
      { B, B, T, B, B },
      { T, B, T, B, B },
      { T, B, T, B, B },      
      { B, B, B, B, B },
      { T, T, T, T, T },
   };
   const short LTE_1bar[CHAR_ROW][CHAR_COL] = {
      { B, B, B, B, B },
      { B, B, B, B, B },
      { B, B, B, B, B },
      { B, B, B, B, B },
      { T, B, B, B, B },
      { T, B, B, B, B },      
      { B, B, B, B, B },
      { T, T, T, T, T },
   };
const short LTE_0bar[CHAR_ROW][CHAR_COL] = {
      { B, B, B, B, B },
      { B, B, B, B, B },
      { B, B, B, B, B },
      { B, B, B, B, B },
      { B, B, B, B, B },
      { B, B, B, B, B },      
      { B, B, B, B, B },
      { T, T, T, T, T },
   };
// array of pointers to all number "bit" masks
const short *numbers[10] = {
   &zero[0][0], &one[0][0], &two[0][0], &three[0][0], &four[0][0], &five[0][0], &six[0][0], &seven[0][0], &eight[0][0], &nine[0][0]
   };

const short *letters[26] = {
   &A_[0][0],&B_[0][0],&C_[0][0],&D_[0][0],&E_[0][0],&F_[0][0],&G_[0][0],&H_[0][0],&I_[0][0],&J_[0][0],&K_[0][0],&L_[0][0],&M_[0][0],
   &N_[0][0],&O_[0][0],&P_[0][0],&Q_[0][0],&R_[0][0],&S_[0][0],&T_[0][0],&U_[0][0],&V_[0][0],&W_[0][0],&X_[0][0],&Y_[0][0],&Z_[0][0],

};




