/*
 * Routine for drawing an image patch from the SD card to the LCD display.
 */



#ifndef _LCD_IMAGE_H
#define _LCD_IMAGE_H
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






#define  MAP_WIDTH  2048
#define  MAP_HEIGHT  2048
#define  LAT_NORTH  5361858l
#define  LAT_SOUTH  5340953l
#define  LON_WEST  -11368652l
#define  LON_EAST  -11333496l




#define DISPLAY_WIDTH  480
#define DISPLAY_HEIGHT 320
#define YEG_SIZE 2048




typedef struct {
  char file_name[50];
  uint16_t ncols;
  uint16_t nrows;
} lcd_image_t;


struct RestDist{
  uint16_t index;
  uint16_t dist;
};

// calling the functions in function.cpp so that they could be used in a1part1.cpp

int32_t  x_to_lon(int16_t x);
int32_t  y_to_lat(int16_t y);
int16_t  lon_to_x(int32_t  lon);
int16_t  lat_to_y(int32_t  lat);
int incrementX(int value);
int incrementY(int value);
int decrementX(int value);
int decrementY(int value);
void swap_ptr(RestDist *ptr1, RestDist *ptr2);
int partition(RestDist *ptr, int len, int pivot);
void isort(RestDist *ptr, int len);
void qsort(RestDist *ptr, int len);
//int partition(RestDist *ptr, int len, int pivot);
/* Draws the referenced image to the LCD screen.
 *
 * img           : the image to draw
 * tft           : the initialized tft struct
 * icol, irow    : the upper-left corner of the image patch to draw
 * scol, srow    : the upper-left corner of the screen to draw to
 * width, height : controls the size of the patch drawn.
 */



void lcd_image_draw(const lcd_image_t *img, MCUFRIEND_kbv *tft,
		    uint16_t icol, uint16_t irow,
		    uint16_t scol, uint16_t srow,
		    uint16_t width, uint16_t height);


#endif
