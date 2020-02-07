


#include "lcd_image.h"
extern bool sentinel;

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

int incrementX(int value){
	Serial.println(value);
  int temp = value;
  if (value >= MAP_WIDTH - 840){
    value = MAP_WIDTH - 420;
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
  Serial.println(value);
  return value;
}

int decrementX(int value){
  int temp = value;
  if (value < 420){
    value = 0;
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
  //Serial.println(value);
  return value;
}


int incrementY(int value){
  int temp = value;
  if (value > 2048 - 640){
    value = 2048 - 320;
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
  //Serial.println(value);
  return value;
}


int decrementY(int value){
  int temp = value;
  if (value < 320){
    value = 0;
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
  //Serial.println(value);
  return value;
}

void swap_ptr(RestDist *ptr1, RestDist *ptr2){

  RestDist temp = *ptr1;

  *ptr1 = *ptr2;

  *ptr2 = temp;

}


void isort(RestDist *ptr, int len){
  int i = 1;

  int j;

  while(i < len){

    j = i;
    // implementing the sorting algorithm, to get the next struct we have to
    // go up by 4.
    while((j > 0) && ((ptr + (j - 1))->dist > (ptr + j)->dist)){

      swap_ptr(ptr + (j - 1), (ptr + j));

      j--;

    }

    i++;
  }
  
}
