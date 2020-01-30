/*
  Template for exercise 2. All required constants and tft/sd/touchscreen
  initializations are here, as is an implementation of getRestaurant
  (the slow version) that should be similar to what is covered in the lecture.

  Use it freely as a starting point for exercise 2.
*/
#include <Arduino.h>


#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <SD.h>
#include <TouchScreen.h>
#include <stdlib.h>
#include "lcd_image.h"

#define SD_CS 10
#define RED 0xF800
// physical dimensions of the tft display (# of pixels)
#define DISPLAY_WIDTH  480
#define DISPLAY_HEIGHT 320

// touch screen pins, obtained from the documentaion
#define YP A3 // must be an analog pin, use "An" notation!
#define XM A2 // must be an analog pin, use "An" notation!
#define YM 9  // can be a digital pin
#define XP 8  // can be a digital pin

// dimensions of the part allocated to the map display
#define MAP_DISP_WIDTH (DISPLAY_WIDTH - 60)
#define MAP_DISP_HEIGHT DISPLAY_HEIGHT

#define REST_START_BLOCK 4000000
#define NUM_RESTAURANTS 1066

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
//  These  functions  convert  between x/y map  position  and  lat/lon// (and  vice  versa.)

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

MCUFRIEND_kbv tft;

uint32_t num = 0;

uint32_t blockNum;


struct RestDist{
  uint16_t index;
  uint16_t dist;
};


RestDist rest_dist[NUM_RESTAURANTS];


// latitude is y
// longtitude is x

// a multimeter reading says there are 300 ohms of resistance across the plate,
// so initialize with this to get more accurate readings
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// different than SD
Sd2Card card;

struct restaurant {
  int32_t lat;
  int32_t lon;
  uint8_t rating; // from 0 to 10
  char name[55];
};


restaurant restBlock[8];
restaurant rest;

void getRestaurantFast(int restIndex, restaurant* restPtr);

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

  // tft display initialization
  uint16_t ID = tft.readID();
  tft.begin(ID);

  tft.fillScreen(TFT_BLACK);
  tft.setRotation(1);

  // SD card initialization for raw reads
  Serial.print("Initializing SPI communication for raw reads...");
  if (!card.init(SPI_HALF_SPEED, SD_CS)) {
    Serial.println("failed! Is the card inserted properly?");
    while (true) {}
  }
  else {
    Serial.println("OK!");
  }

  // printinrestaurant restBlock[8];g the information on the tft display

}

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

int findResaurant(int x, int y){
  for (int i = 0; i < NUM_RESTAURANTS; ++i) {
    getRestaurantFast(i, &rest);
    rest_dist[i].index = i;
    int my_x = x_to_lon(x) - rest.lon;
    int my_y = y_to_lat(y) - rest.lat;
    rest_dist[i].dist = abs(my_x) + abs(my_y);
    // not sure to cast them into doubles yet.
  }
}

// the implementation from class
void getRestaurant(int restIndex, restaurant* restPtr) {
  uint32_t blockNum = REST_START_BLOCK + restIndex/8;
  restaurant restBlock[8];

  while (!card.readBlock(blockNum, (uint8_t*) restBlock)) {
    Serial.println("Read block failed, trying again.");
  }

  *restPtr = restBlock[restIndex % 8];
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

int main() {
  
  setup();

  Serial.end();  

  return 0;
}