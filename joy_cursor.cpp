/*
	Demonstrating cursor movement over the map of Edmonton. You will improve over
  what we do in the next weekly exercise.
*/

#define SD_CS 10
#define JOY_VERT  A9 // should connect A9 to pin VRx
#define JOY_HORIZ A8 // should connect A8 to pin VRy
#define JOY_SEL   53

#include <Arduino.h>

// core graphics library (written by Adafruit)
#include <Adafruit_GFX.h>

// Hardware-specific graphics library for MCU Friend 3.5" TFT LCD shield
#include <MCUFRIEND_kbv.h>

// LCD and SD card will communicate using the Serial Peripheral Interface (SPI)
// e.g., SPI is used to display images stored on the SD card
#include <SPI.h>

// needed for reading/writing to SD card
#include <SD.h>

#include "lcd_image.h"


MCUFRIEND_kbv tft;

#define DISPLAY_WIDTH  480
#define DISPLAY_HEIGHT 320
#define YEG_SIZE 2048

lcd_image_t yegImage = { "yeg-big.lcd", YEG_SIZE, YEG_SIZE };

#define JOY_CENTER   512
#define JOY_FAST 280
#define JOY_DEADZONE 64

#define CURSOR_SIZE 9



// the cursor position on the display
int cursorX, cursorY;

bool sentinel = false;

int map_center[2] = {YEG_SIZE/2 - (DISPLAY_WIDTH - 60)/2, YEG_SIZE/2 - DISPLAY_HEIGHT/2};

int incrementX(int value){
  int temp = value;
  if (value > 2048 - 420){
    value = 2048 - 210;
  }
  else{
    value += 420;
  }
  if(temp == value){
    sentinel = false;
  }
  else{
    sentinel = true;
  }
  return value;
}

int decrementX(int value){
  int temp = value;
  if (value < 420){
    value = 210;
  }
  else{
    value -= 420;
  }
  if(temp == value){
    sentinel = false;
  }
  else{
    sentinel = true;
  }
  return value;
}


int incrementY(int value){
  int temp = value;
  if (value > 2048 - 320){
    value = 2048 - 210;
  }
  else{
    value += 320;
  }
  if(temp == value){
    sentinel = false;
  }
  else{
    sentinel = true;
  }
  return value;
}


int decrementY(int value){
  int temp = value;
  if (value < 320){
    value = 210;
  }
  else{
    value -= 320;
  }
  if(temp == value){
    sentinel = false;
  }
  else{
    sentinel = true;
  }
  return value;
}


// forward declaration for redrawing the cursor
void redrawCursor(uint16_t colour);

void setup() {
  init();

  Serial.begin(9600);

	pinMode(JOY_SEL, INPUT_PULLUP);

	//    tft.reset();             // hardware reset
  uint16_t ID = tft.readID();    // read ID from display
  Serial.print("ID = 0x");
  Serial.println(ID, HEX);
  if (ID == 0xD3D3) ID = 0x9481; // write-only shield
  
  // must come before SD.begin() ...
  tft.begin(ID);                 // LCD gets ready to work

	Serial.print("Initializing SD card...");
	if (!SD.begin(SD_CS)) {
		Serial.println("failed! Is it inserted properly?");
		while (true) {}
	}
	Serial.println("OK!");

	tft.setRotation(1);

  tft.fillScreen(TFT_BLACK);

  // draws the centre of the Edmonton map, leaving the rightmost 60 columns black

	lcd_image_draw(&yegImage, &tft, map_center[0], map_center[1],
                 0, 0, DISPLAY_WIDTH - 60, DISPLAY_HEIGHT);

  // initial cursor position is the middle of the screen
  cursorX = (DISPLAY_WIDTH - 60)/2;
  cursorY = DISPLAY_HEIGHT/2;

  redrawCursor(TFT_RED);
}

void redrawCursor(uint16_t colour) {
  tft.fillRect(cursorX - CURSOR_SIZE/2, cursorY - CURSOR_SIZE/2,
               CURSOR_SIZE, CURSOR_SIZE, colour);
}

void processJoystick() {
  int xVal = analogRead(JOY_HORIZ);
  int yVal = analogRead(JOY_VERT);
  int buttonVal = digitalRead(JOY_SEL);

  
  // Record the position before joy stick is pushed
  int pt_x = cursorX;
  int pt_y = cursorY;
  //while loop is used to prevent flickering which checks 4 cases.
  while (analogRead(JOY_HORIZ) - JOY_CENTER < JOY_DEADZONE && analogRead(JOY_VERT) - JOY_CENTER < JOY_DEADZONE &&
  	JOY_CENTER - analogRead(JOY_HORIZ) < JOY_DEADZONE  && JOY_CENTER - analogRead(JOY_VERT)  < JOY_DEADZONE){
  	Serial.println("a");
    delay(20);
  }
  // now move the cursor
  // cursor's speed is relative to how much the joy stick is pushed

  if (yVal < JOY_CENTER - JOY_DEADZONE){
  	cursorY -= (JOY_CENTER - yVal)/64;
  }
  else if (yVal > JOY_CENTER + JOY_DEADZONE){
  	cursorY += (yVal - JOY_CENTER)/64;
  }
  if (xVal < JOY_CENTER - JOY_DEADZONE){
  	cursorX += (JOY_CENTER - xVal)/64;
  }
  else if (xVal > JOY_CENTER + JOY_DEADZONE){
  	cursorX -= (xVal - JOY_CENTER)/64;
  }

  Serial.print("x: ");

  Serial.println(cursorX);

  Serial.print("y: ");

  Serial.println(cursorY);

  // Getting the coordinates to draw another patch
  
  if(cursorX >= 420){
    map_center[0] = incrementX(map_center[0]);
  }

  else if(cursorX <= 0){
    map_center[0] = decrementX(map_center[0]);
  }

  if(cursorY >= 320){
    map_center[1] = incrementY(map_center[1]); 
  }

  else if(cursorY <= 0){
    map_center[1] = decrementY(map_center[1]);
  }

  if(sentinel){
  lcd_image_draw(&yegImage, &tft, map_center[0], map_center[1],
  0, 0, DISPLAY_WIDTH - 60, DISPLAY_HEIGHT);
  cursorX = 210;
  cursorY = 160;
  sentinel = false;
  }

  lcd_image_draw(&yegImage, &tft,
  // choose icol and irow which are the positions of previous point image
  map_center[0] + pt_x - CURSOR_SIZE/2, map_center[1]+ pt_y - CURSOR_SIZE/2,
  // place the redrawn image on the correct position
  pt_x - CURSOR_SIZE/2, pt_y - CURSOR_SIZE/2,
  // measurements of the redrawn image which is the same size as the cursor
  CURSOR_SIZE, CURSOR_SIZE);
  

  //Constrain function helps to limit the boundaries of a variable
  cursorX = constrain(cursorX, 0 + float(CURSOR_SIZE)/2.0, 420 - float(CURSOR_SIZE)/2.0);

  cursorY = constrain(cursorY, 0 + float(CURSOR_SIZE)/2.0, 320 - float(CURSOR_SIZE)/2.0);
  
  // draw a small patch of the Edmonton map at the old cursor position before
  // drawing a red rectangle at the new cursor position
  
  lcd_image_draw(&yegImage, &tft,
  	// choose icol and irow which are the positions of previous point image
   map_center[0] + pt_x - CURSOR_SIZE/2, map_center[1]+ pt_y - CURSOR_SIZE/2,
   	// place the redrawn image on the correct position
   pt_x - CURSOR_SIZE/2, pt_y - CURSOR_SIZE/2,
   	// measurements of the redrawn image which is the same size as the cursor
   CURSOR_SIZE, CURSOR_SIZE);
   //}
  
  redrawCursor(TFT_RED);

  delay(20);
}

int main() {
	setup();

  while (true) {
    processJoystick();
  }

	Serial.end();
	return 0;
}
