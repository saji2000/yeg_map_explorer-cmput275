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

#include <TouchScreen.h>
#define MAP_DISP_WIDTH (DISPLAY_WIDTH - 60)
#define MAP_DISP_HEIGHT DISPLAY_HEIGHT

#define REST_START_BLOCK 4000000
#define NUM_RESTAURANTS 1066

#define RED 0xF800

// calibration data for the touch screen, obtained from documentation
// the minimum/maximum possible readings from the touch point
#define TS_MINX 100
#define TS_MINY 120
#define TS_MAXX 940
#define TS_MAXY 920

// thresholds to determine if there was a touch
#define MINPRESSURE   10
#define MAXPRESSURE 1000

//  These  constants  are  for  the  2048 by 2048  map.
#define  MAP_WIDTH  2048
#define  MAP_HEIGHT  2048
#define  LAT_NORTH  5361858l
#define  LAT_SOUTH  5340953l
#define  LON_WEST  -11368652l
#define  LON_EAST  -11333496l




#define DISPLAY_WIDTH  480
#define DISPLAY_HEIGHT 320
#define YEG_SIZE 2048

lcd_image_t yegImage = { "yeg-big.lcd", YEG_SIZE, YEG_SIZE };

#define JOY_CENTER   512
#define JOY_FAST 280
#define JOY_DEADZONE 64

#define CURSOR_SIZE 9

MCUFRIEND_kbv tft;

// the cursor position on the display
int cursorX, cursorY;

bool sentinel = false;

int map_center[2] = {YEG_SIZE/2 - (DISPLAY_WIDTH - 60)/2, YEG_SIZE/2 - DISPLAY_HEIGHT/2};

int32_t  x_to_lon(int16_t x) {

  return  map(x, 0, MAP_WIDTH , LON_WEST , LON_EAST);

}

int32_t  y_to_lat(int16_t y) {

  return  map(y, 0, MAP_HEIGHT , LAT_NORTH , LAT_SOUTH);

}

int16_t  lon_to_x(int32_t  lon) {
  return  map(lon , LON_WEST , LON_EAST , 0, MAP_WIDTH);
}


int16_t  lat_to_y(int32_t  lat) {
  return  map(lat , LAT_NORTH , LAT_SOUTH , 0, MAP_HEIGHT);
}

uint32_t num = 0;

uint32_t blockNum;

Sd2Card card;

struct restaurant {
  int32_t lat;
  int32_t lon;
  uint8_t rating; // from 0 to 10
  char name[55];
};

restaurant restBlock[8];
restaurant rest;


struct RestDist{
  uint16_t index;
  uint16_t dist;
};


//TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

RestDist rest_dist[NUM_RESTAURANTS];

RestDist *rest_d = rest_dist;


void swap_ptr(RestDist *ptr1, RestDist *ptr2){

  RestDist *temp = ptr1;

  ptr1 = ptr2;

  ptr2 = temp;

}


void isort(RestDist *ptr, int len){
  int i = 1;

  int j;

  while(i < len){

    j = i;
    // implementing the sorting algorithm, to get the next struct we have to
    // go up by 4.
    while((j > 0) && ((ptr + 4*(j - 1))->dist > (ptr + 4*j)->dist)){

      swap_ptr((ptr + 4*(j - 1))->dist, (ptr + 4*j)->dist);

      j--;

    }

    i++;

  }
}


void getRestaurantFast(int restIndex, restaurant* restPtr) {
  // todo
  blockNum = REST_START_BLOCK + restIndex/8;
  
  // we use num to check if the index is within the same block or not
  if(num != blockNum){
    num = blockNum;
    while (!card.readBlock(blockNum, (uint8_t*) restBlock)) {
      Serial.println("Read block failed, trying again.");
    }
  }
  *restPtr = restBlock[restIndex % 8];
}

int findResaurant(int x, int y){
  for (int i = 0; i < NUM_RESTAURANTS; ++i) {
    getRestaurantFast(i, &rest);
    rest_dist[i].index = i;
    int my_x = x_to_lon(x) - rest.lon;
    int my_y = y_to_lat(y) - rest.lat;
    rest_dist[i].dist = abs(my_x) + abs(my_y);
    // not sure to cast them into doubles yet.
    Serial.println("sup");
  }
}

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

void display(){

  tft.fillScreen (0);tft.setCursor(0, 0); //  where  the  characters  will be  displayed

  tft.setTextWrap(false);

  int selectedRest = 0; //  which  restaurant  is  selected?

  for (int16_t i = 0; i < 21; i++) {
    restaurant r;
    getRestaurantFast(rest_dist[i].index , &r);

    if (i !=  selectedRest) { // not  highlighted//  white  characters  on  black  background
      tft.setTextColor (0xFFFF , 0x0000);
    } 

    else { //  highlighted//  black  characters  on  white background
      tft.setTextColor (0x0000 , 0xFFFF);
    }

    tft.print(r.name);

    tft.print("\n");
  }

  tft.print("\n");

}

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

  
  
  // Record the position before joy stick is pushed
  int pt_x = cursorX;
  int pt_y = cursorY;
  //while loop is used to prevent flickering which checks 4 cases.
  while (analogRead(JOY_HORIZ) - JOY_CENTER < JOY_DEADZONE && analogRead(JOY_VERT) - JOY_CENTER < JOY_DEADZONE &&
  	JOY_CENTER - analogRead(JOY_HORIZ) < JOY_DEADZONE  && JOY_CENTER - analogRead(JOY_VERT)  < JOY_DEADZONE && digitalRead(JOY_SEL)){  	
    delay(20);
  }

  if(digitalRead(JOY_SEL) == 0){
    findResaurant(map_center[0] + cursorX, map_center[1] + cursorY);
    isort(rest_d, NUM_RESTAURANTS);
    display();
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
