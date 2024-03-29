
#define SD_CS 10
#define JOY_VERT  A9 // should connect A9 to pin VRx
#define JOY_HORIZ A8 // should connect A8 to pin VRy
#define JOY_SEL   53

#include "lcd_image.h"

#include <TouchScreen.h>


#define MAP_DISP_WIDTH (DISPLAY_WIDTH - 60)
#define MAP_DISP_HEIGHT DISPLAY_HEIGHT

#define REST_START_BLOCK 4000000
#define NUM_RESTAURANTS 1066

#define RED 0xF800
#define BLUE 0x001F

#define YP A3 // must be an analog pin, use "An" notation!
#define XM A2 // must be an analog pin, use "An" notation!
#define YM 9  // can be a digital pin
#define XP 8  // can be a digital pin

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


lcd_image_t yegImage = { "yeg-big.lcd", YEG_SIZE, YEG_SIZE };

#define JOY_CENTER   512
#define JOY_FAST 280
#define JOY_DEADZONE 64

#define CURSOR_SIZE 9

MCUFRIEND_kbv tft;

// the cursor position on the display
int cursorX, cursorY;
bool sentinel = false;
int clicked;
int star = 5;
int sort = 1;
int j = 0;

// map_corner holds the top left corner value of the map
int map_center[2] = {YEG_SIZE/2 - (DISPLAY_WIDTH - 60)/2, YEG_SIZE/2 - DISPLAY_HEIGHT/2};

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

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

TSPoint p = ts.getPoint();
 
RestDist rest_dist[NUM_RESTAURANTS];

RestDist *rest_d = rest_dist;

int rate(restaurant r){
  int rating = floor(double(r.rating + 1)/2);
  rating = constrain(rating, 1, 5);
  return rating;
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

void pressed(){
  // drawing the points on where restaurants are located at
  for (int i = 0; i < NUM_RESTAURANTS; ++i){
    getRestaurantFast(i, &rest);
    int circle_x = lon_to_x(rest.lon) - map_center[0];
    int circle_y = lat_to_y(rest.lat) - map_center[1];
    int r = rate(rest);
    if(circle_x < 417 && (r >= star)){
      tft.drawCircle(circle_x, circle_y, 3, BLUE);
      tft.fillCircle(circle_x, circle_y, 3, BLUE);
    }
  }
}


void findResaurant(int x, int y){
  // finds the 21st closest restaurants to the point
  j = 0;
  int temp;
  for (int i = 0; i < NUM_RESTAURANTS; ++i) {
    getRestaurantFast(i, &rest);
    temp = rate(rest);
    // Serial.print("restaurant");
    // Serial.println(r);
    // Serial.print("star:");
    // Serial.println(star);
    Serial.println(i);
    if (temp >= star){
      rest_dist[j].index = i;
      int my_x = x_to_lon(x) - rest.lon;
      int my_y = y_to_lat(y) - rest.lat;
      rest_dist[j].dist = abs(my_x) + abs(my_y);
      j++;
    }
  }
}



// forward declaration for redrawing the cursor
void redrawCursor(uint16_t colour);

void display(int selectedRest){

  // nt selectedRest = 0; //  which  restaurant  is  selected?
  clicked += selectedRest;

  clicked = constrain(clicked, 0, 19);
  if(selectedRest == 0){
    tft.fillScreen (0);tft.setCursor(0, 0); //  where  the  characters  will be  displayed

    tft.setTextWrap(false);

    tft.setTextSize(2);
    for (int16_t i = 0; i < 21; i++) {

      restaurant r;
      getRestaurantFast(rest_dist[i].index , &r);

      if (i !=  0) { // not  highlighted//  white  characters  on  black  background
        tft.setTextColor (0xFFFF , 0x0000);
      } 

      else { //  highlighted//  black  characters  on  white background
        tft.setTextColor (0x0000 , 0xFFFF);
      }

      tft.print(r.name);

      tft.print("\n");
      
    }
  }
  int my_num;
  if(selectedRest > 0){
        // cursor pushed up, unhighlight the old one and highlight the new one
        restaurant s;
        getRestaurantFast(rest_dist[clicked - 1].index , &s);
        my_num = 16 * (clicked - 1);
        tft.setCursor(0, my_num);
        tft.print(s.name);
        //tft.print("\n");
        //Serial.println(clicked);
        
        tft.setTextColor (0x0000 , 0xFFFF);

        getRestaurantFast(rest_dist[clicked].index , &s);
        tft.setCursor(0, 16 * clicked);
        tft.print(s.name);
        tft.setTextColor (0xFFFF , 0x0000);        

  }
  else if(selectedRest < 0){

        // cursor pushed down, unhighlight the old one and highlight the new one

        restaurant s;
        getRestaurantFast(rest_dist[clicked + 1].index , &s);
        my_num = 16 * (clicked + 1); 
        tft.setCursor(0, my_num);
        tft.print(s.name);
        tft.setTextColor (0x0000 , 0xFFFF);


        getRestaurantFast(rest_dist[clicked].index , &s);
        my_num = 16 * (clicked);
        tft.setCursor(0, my_num);
        tft.print(s.name);
        //tft.print("\n");
        Serial.println(clicked);
        tft.setTextColor (0xFFFF , 0x0000);
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

  Serial.print("Initializing SPI communication for raw reads...");
  if (!card.init(SPI_HALF_SPEED, SD_CS)) {
    Serial.println("failed! Is the card inserted properly?");
    while (true) {}
  }
  else {
    Serial.println("OK!");
  }

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

  //while loop is used to prevent flickering which checks 6 cases.
  while (analogRead(JOY_HORIZ) - JOY_CENTER < JOY_DEADZONE && analogRead(JOY_VERT) - JOY_CENTER < JOY_DEADZONE &&
  	JOY_CENTER - analogRead(JOY_HORIZ) < JOY_DEADZONE  && JOY_CENTER - analogRead(JOY_VERT)  < JOY_DEADZONE && 
    digitalRead(JOY_SEL) && p.z <= MINPRESSURE){  	
    delay(20);
    p = ts.getPoint();

    pinMode(YP, OUTPUT); 
    pinMode(XM, OUTPUT);

    // !!!! Create a function such that if any of those buttons is pressed 
    // !!!! it changes the the button (have global variable for the sort and stars)

  }

  p = ts.getPoint();
  // reading the touchscreen values. (only pressure is important)
  pinMode(YP, OUTPUT); 
  pinMode(XM, OUTPUT);

  if (p.z > MINPRESSURE){
    pressed();
  }

  int flag = digitalRead(JOY_SEL);
  bool flag1 = true;

  // Mode1 starts when the joystick button is pressed
  while(!flag){
    if(flag1){
      // Entering Mode1
      findResaurant(map_center[0] + cursorX, map_center[1] + cursorY);
      //isort(rest_d, NUM_RESTAURANTS);
      qsort(rest_d, j);
      display(0);
      flag1 = false;
    }

    if(analogRead(JOY_VERT) - JOY_CENTER > JOY_DEADZONE){
      // cursor pushed down
      display(1);
    }
    else if(JOY_CENTER - analogRead(JOY_VERT) > JOY_DEADZONE){
      // cursor pushed up
      display(-1);
    }
    
    if(digitalRead(JOY_SEL) == 0){
      // drawing the new patch of yeg map at the position of a selected 
      // restaurant
      flag = true;
      restaurant rest;
      getRestaurantFast(rest_dist[clicked].index, &rest);
      map_center[0] = lon_to_x(rest.lon) - (DISPLAY_WIDTH - 60)/2;
      map_center[1] = lat_to_y(rest.lat) - DISPLAY_HEIGHT/2;

      map_center[0] = constrain(map_center[0], 0, MAP_WIDTH - MAP_DISP_WIDTH);

      map_center[1] = constrain(map_center[1], 0, MAP_HEIGHT - MAP_DISP_HEIGHT);

      lcd_image_draw(&yegImage, &tft, map_center[0], map_center[1],
      0, 0, DISPLAY_WIDTH - 60, DISPLAY_HEIGHT);

      cursorX = lon_to_x(rest.lon) - map_center[0];
      cursorY = lat_to_y(rest.lat) - map_center[1];

      tft.drawRect(MAP_DISP_WIDTH, 0, 60, MAP_DISP_HEIGHT, TFT_BLACK);
      tft.fillRect(MAP_DISP_WIDTH, 0, 60, MAP_DISP_HEIGHT, TFT_BLACK);
      Serial.println("it's here");
      // cursorX = MAP_DISP_WIDTH/2;
      // cursorY = MAP_DISP_HEIGHT/2;
      }
    }
  
  clicked = 0;
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
  Serial.println("wtf");
  while(true){
    processJoystick();
  }

	Serial.end();
	return 0;
}
