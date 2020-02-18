// I have included some of the functions here to organize the code better

#include "lcd_image.h"
extern bool sentinel;

int32_t  x_to_lon(int16_t x) {
	// conversion of x position to longtitude
  return  map(x, 0, MAP_WIDTH , LON_WEST , LON_EAST);

}

int32_t  y_to_lat(int16_t y) {
// conversion of y position to latitude
  return  map(y, 0, MAP_HEIGHT , LAT_NORTH , LAT_SOUTH);

}

int16_t  lon_to_x(int32_t  lon) {
	// conversion of longtitude to x position
  return  map(lon , LON_WEST , LON_EAST , 0, MAP_WIDTH);
}


int16_t  lat_to_y(int32_t  lat) {
	// conversion of latitude to y position
  return  map(lat , LAT_NORTH , LAT_SOUTH , 0, MAP_HEIGHT);
}

int incrementX(int value){
	// Increment the x position of new patch 
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
	// decrement the x position of new patch 
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
	// Increment the y position of new patch 
  int temp = value;
  if (value > MAP_WIDTH - 640){
    value = MAP_WIDTH - 320;
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
	// decrement the y position of new patch 
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
	// implemented function for isort

  auto temp = *ptr1;

  *ptr1 = *ptr2;

  *ptr2 = temp;

}


void isort(RestDist *ptr, int len){
	// given algorthim for isort to sort the restaurants based on their manhattan
	// distance to the cursor
  int i = 1;

  int j;

  while(i < len){

    j = i;
    // implementing the sorting algorithm, to get the next struct we have to
    // go up by 4.(c++ does this on its own)
    while((j > 0) && ((ptr + (j - 1))->dist > (ptr + j)->dist)){

      swap_ptr(ptr + (j - 1), (ptr + j));

      j--;

    }

    i++;
  }
  
}

// void qsort(RestDist a[], uint16_t len){
//   if (len <= 1) return;

//   uint16_t p = (len + 1)/2;

//   swap_ptr(&a[p], &a[len - 1]);

//   uint16_t low = 0;
//   uint16_t high = len - 2;

//   while(high >= low){
//     if(a[high].dist <= a[len-1].dist){
//       high--;
//     }
//     else if(a[low].dist <= a[len-1].dist){
//       low--;
//     }
//     else{
//       swap_ptr(&a[low], &a[high]);
//     }
//   }

//   swap_ptr(&a[low], &a[len-1]);

//   qsort(a, low);
//   qsort(a + low, len - low);
// }



int partition(RestDist *ptr, int len, int pivot){
  // This is partition function which does the sorting for q sort
  swap_ptr(ptr + pivot, ptr + len - 1);
  // pivot is chosen to be the last element, can be anything
  //Serial.println(pivot);
  int low = 0;
  int high = len - 2;
  while(high >= low){
    if((ptr + low)->dist <= (ptr + len - 1)->dist){
      low++;
    }
    else if((ptr + high)->dist > (ptr + len - 1)->dist){
      high--;
    }
    else{
      swap_ptr(ptr + low, ptr + high);
      low++;
    }    
  }
  
  swap_ptr(ptr + low, ptr + len - 1);
  return low;
}

void qsort(RestDist *ptr, int len){
  if(len <= 1) return;

    int p = partition(ptr, len, len/2);

    qsort(ptr, p);
    qsort(ptr + p + 1, len - p - 1);
  
}
