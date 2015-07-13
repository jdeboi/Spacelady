/*********************************************************
SPACELADY
an interactive, Arduino-controlled Neopixel mural

June 2015
Jenna deBoisblanc
http://jdeboi.com


This code uses libraries and examples from Adafruit, specifically for
the MPR121 Breakout and Neopixels. Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit! ----> https://www.adafruit.com/products/

**********************************************************/
#include <math.h>
///////////////////////////////
// CAPACITIVE
#include <Wire.h>
#include "Adafruit_MPR121.h"
Adafruit_MPR121 cap = Adafruit_MPR121();
uint16_t lasttouched = 0;
uint16_t currtouched = 0;
int currentPressed = 0;
#define BOUNCE        0
#define PULSING       1
#define SPARKLES      2
#define WIPE          3
#define FILLUP        4
#define UNFILL        5
// rainbowCycle
#define RANDBRIGHT    6
#define BLOCKS        7
// rainbowBounce
#define END           8
int currentMode = 1;
long soundT = 0;
boolean firstSound = true;

///////////////////////////////
// NEOPIXELS
#include <Adafruit_NeoPixel.h>
#include <avr/power.h>
#define NEOPIN 2
#define NEOPIN2 3
#define NUMNEO 13
#define NUMNEO2 9
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMNEO, NEOPIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(NUMNEO2, NEOPIN2, NEO_GRB + NEO_KHZ800);
uint8_t currentWheelPos = 0;
uint32_t noteColors[4];
uint8_t notePositions[4] = {5, 54, 150, 230};
uint8_t noteBright[22];
boolean darkOn = false;
boolean sparkleOn = false;
int previousStart;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void setup() { 
  initializeColors();  
  Serial.begin(9600);
  cap.begin(0x5A);
  strip.begin();
  strip2.begin();
  strip.show();
  strip2.show();
}

void loop() {
  updateKeys();                      // update key positions
  checkPresses();                    // any keys pressed?
  if(currentMode != WIPE && currentMode != FILLUP && currentMode != UNFILL && currentMode != BLOCKS && currentMode != END) {
    if(allKeysOff()) setAllPixels(0);  // reset Neopixels if no presses
  }
}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

///////////////////////////////
// PROGRAM LOGIC

void checkPresses() {
  int pins[] = {10,7,6,9,8,5,1,0,3,2,4};     
  if(keyPressed(pins[4])) shiftMode();    // change mode button
  else if(currentMode == END) {
    for(int k=6; k<10; k++) {
      if(keyPressed(pins[k])) {
        if(k==9) {
          pulseOut(noteColors[k-6],10);
          return;
        }
        else {
          fadeIn(noteColors[k-6], 10);
          break;
        }
      }
      else if(keyReleased(pins[k])) {
        if(k != 9) fadeOut(noteColors[k-6],10);
        break;
      }
    }  
  }
  else if (currentMode == FILLUP || currentMode == UNFILL) {
    for(int k=0; k<4; k++) {
      if(keyPressed(pins[k])) {
        if(currentMode == FILLUP) fillUpColorKey(k,50);
        else unFillColorKey(k,50);
        break;
      }
    } 
    for(int k=5; k<10; k++) {
      if(keyPressed(pins[k])) {
        if(currentMode == FILLUP) fillUpColorKey(k-1,50);
        else unFillColorKey(k-1,50);
        break;
      }
    } 
  } 
  else {   
    // "Space, space lady..."
    if(keyPressed(pins[0])) playSound(0);
    else if(keyPressed(pins[1])) playSound(1);
    else if(keyPressed(pins[2])) playSound(2);
    
    // control buttons
    else if(keyPressed(pins[3])) {
      while(1) rainbowCycleOrig(0);
    }
    // rainbow buttons
    else if(keyPressed(pins[5])) rainbowBounce(30);
    else if(keyPressed(pins[10])) rainbowCycle(5);
    
    // color keys
    for(int k=6; k < 10; k++) {
      if(keyPressed(pins[k])) {
        playColor(k-6);
        break;
      }
    }
  }
}

void shiftMode() {
  currentMode++;
  //Serial.print("mode: "); Serial.println(currentMode);
  if(currentMode == 0) {
    strip.setBrightness(150);
    strip2.setBrightness(150);
  }
  else if(currentMode > 8) currentMode = 0;
}

void playColor(int key) {
  switch(currentMode) {  
    case BOUNCE:
      //Serial.println("bounce!");
      bounce(noteColors[key],key, 25);
      break;
    case PULSING:
      //Serial.println("pulse!");
      pulse(noteColors[key],key, 5);
      break;
    case SPARKLES:
      //Serial.println("sparkles!");
      theaterChaseDouble(key, 50);
      break;
    case WIPE:
      //Serial.println("wipe!");
      colorWipe(noteColors[key],key,20);
      break;
    case RANDBRIGHT:
      //Serial.println("random brightness!");
      randomBrightRamp(key, 5);
      break;
    case BLOCKS:
      //Serial.println("wipe!");
      blocks(key);
      break;
  }
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

///////////////////////////////
// CAPACITIVE

boolean keyPressed(int num) {
  if ((currtouched & _BV(num)) && !(lasttouched & _BV(num)) ) {
    //Serial.println(num);
    return true;
  }
  return false;
}

boolean keyReleased(int num) {
   if (!(currtouched & _BV(num)) && (lasttouched & _BV(num)) ) {
     // Serial.print("released: "); Serial.println(num);
     return true;
   }
   return false;
}

boolean allKeysOff() {
  for(int i = 0; i < 11; i++) {
    if (currtouched & (1 << i)) return false;
  }
  return true;
}

void updateKeys() {
  lasttouched = currtouched;
  currtouched = cap.touched();
}

boolean colorKeyUp(uint8_t k) {
  // 0,1,2,3,4,5,6,7,8,9,10
  //10,7,6,9,8,5,1,0,3,2,4 
  int pins[] = {1,0,3,2,5,4};
  if (!(cap.touched() & (1 << pins[k]))) return true;
  return false;
}

void checkSound() {
  int pins[] = {10, 7, 6};
  uint16_t temp = cap.touched();
  for(int i = 0; i < 3; i++) {
    if ((temp & _BV(pins[i])) && !(lasttouched & _BV(pins[i])) ) {
      //Serial.print("Playing sound "); Serial.println(i);
      playSound(i);
      return;
    }
  }
}

void soundDelay(uint16_t timeDelay) {
  long t = millis();
  while(millis() - t < timeDelay) checkSound();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

///////////////////////////////
// NEOPIXELS
void setNeopixel(int index, uint32_t col) {
  if(index < strip2.numPixels() && index >= 0) {
    strip2.setPixelColor(strip2.numPixels()-1-index, col);
  }
  else if(index > strip2.numPixels()-1 && index < strip2.numPixels()+strip.numPixels()) {
    strip.setPixelColor(index - strip2.numPixels(), col);
  }
}

uint32_t getNeopixelColor(int index) {
  if(index < strip2.numPixels() && index >= 0) {
    return strip2.getPixelColor(strip2.numPixels()-1-index);
  }
  else if(index > strip2.numPixels()-1 && index < strip2.numPixels()+strip.numPixels()) {
    return strip.getPixelColor(index - strip2.numPixels());
  }
}

void setAllPixels(uint32_t col) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, col);
  }
  for(uint16_t i=0; i<strip2.numPixels(); i++) {
    strip2.setPixelColor(i, col);
  }
  strip.show();
  strip2.show();
}

void setPixelBrightness(uint8_t index, uint8_t r, uint8_t g, uint8_t b, uint8_t bright) {
  double hslArray[3];
  int rgbArray[3];
  rgb2HSL(r,g,b, hslArray);
  double lum = bright/256.0;
  hslArray[2]= lum;
  Serial.print("lum: " ); Serial.println(hslArray[2]);
  hsl2RGB(hslArray[0], hslArray[1], hslArray[2], rgbArray);
  setNeopixel(index, strip.Color(rgbArray[0],rgbArray[1],rgbArray[2]));
}

void setPixelBrightness(uint8_t index, uint32_t col, uint8_t bright) {
  byte r = byte(col >> 16);
  byte g = byte(col >> 8);
  byte b = byte(col);
  double hslArray[3];
  int rgbArray[3];
  rgb2HSL(r,g,b, hslArray);
  double lum = bright/256.0;
  hslArray[2]= lum;
  hsl2RGB(hslArray[0], hslArray[1], hslArray[2], rgbArray);
  setNeopixel(index, strip.Color(rgbArray[0],rgbArray[1],rgbArray[2]));
}

void rainbowMix(int key, uint8_t timeDelay) {
  if(key < 4) {
    int range = 256/8-10;
    int startVal = key*256/8;
    int endVal = startVal + range;
    uint16_t i, j;
    
    for(j=startVal; j<endVal; j++) { 
      if(colorKeyUp(key)) return;
      for(i=0; i< strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel(((i * range / strip.numPixels()) + j) & 255));
      }
      for(i=0; i< strip2.numPixels(); i++) {
        strip2.setPixelColor(i, Wheel(((i * range / strip2.numPixels()) + j) & 255));
      }
      strip.show();
      strip2.show();
      soundDelay(timeDelay);
    }
    for(j=endVal; j>startVal; j--) { 
      if(colorKeyUp(key)) return;
      for(i=0; i< strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel(((i * range / strip.numPixels()) + j) & 255));
      }
      for(i=0; i< strip2.numPixels(); i++) {
        strip2.setPixelColor(i, Wheel(((i * range / strip2.numPixels()) + j) & 255));
      }
      strip.show();
      strip2.show();
      soundDelay(timeDelay);
    }
  }
  else {}
}

void fillUpColorKey(uint8_t key, uint8_t timeDelay) {
  strip.clear();
  strip2.clear();
  int totalNeo = strip.numPixels() + strip2.numPixels();
  float fillPixNum = (totalNeo)/8;  // number of pixels per fill
  int startPix = int((key*1.0) * fillPixNum);
  int endPix = int((key+1.0) * fillPixNum);
  int wheelInc = int(256.0/totalNeo);
  //Serial.println(wheelInc);
  int startInc = 0;
  for(int i = 0; i < startPix; i++) {
    setNeopixel(i, Wheel(wheelInc*i + startInc));
  }
  if(key == 7) {
    for(int i = startPix; i < endPix+1; i++) {
      setNeopixel(i, Wheel(wheelInc*i + startInc));
      strip.show();
      strip2.show();
      delay(timeDelay);
    }
  }
  else if(key == 8) {
    for(int i = startPix; i < totalNeo; i++) {
      setNeopixel(i, Wheel(wheelInc*i + startInc));
      strip.show();
      strip2.show();
      delay(timeDelay);
    }
  }
  else { 
    for(int i = startPix; i <= endPix; i++) {
      setNeopixel(i, Wheel(wheelInc*i + startInc));
      strip.show();
      strip2.show();
      delay(timeDelay);
    }
  }
}

void unFillColorKey(uint8_t key, uint8_t timeDelay) {
  int totalNeo = strip.numPixels() + strip2.numPixels();
  float fillPixNum = (totalNeo)/7;  // number of pixels per fill
  int startPix = int((key)* 1.0 * fillPixNum);
  int endPix = int((key+1.0) * fillPixNum);
  int wheelInc = int(256.0/totalNeo);
  int startInc = 0;
  for(int i = 0; i < totalNeo; i++) setNeopixel(i, Wheel(wheelInc*i + startInc));  // all on
  if (key == 8) { 
    strip.show();
    strip2.show();
  }
  else {
    for(int i = totalNeo - 1; i > endPix; i--) setNeopixel(i, 0);
    for(int i = endPix; i >= startPix; i--) {
      setNeopixel(i, 0);
      strip.show();
      strip2.show();
      delay(timeDelay);
    }
  }
}

void lightening(uint32_t col,uint8_t key, uint8_t timeDelay) {
  setAllPixels(0);
  while(!colorKeyUp(key)) {
    checkSound();
    for(int i = 22; i >= -5; i--) {
      if(colorKeyUp(key)) return;
      for(int j = 0; j < 5; j++) {
        setNeopixel(i, col);
      }
      setNeopixel(i+5, 0);
      strip.show();
      strip2.show();
      soundDelay(timeDelay);
    }
  }
}

void rainbowLightening(int timeDelay) {
  setAllPixels(0);
  for(int i = 22; i >= -5; i--) {
    if(colorKeyUp(4)) break;
    setNeopixel(i, Wheel(240));
    setNeopixel(i+1, Wheel(200));
    setNeopixel(i+2, Wheel(150));
    setNeopixel(i+3, Wheel(100));
    setNeopixel(i+4, Wheel(50));
    setNeopixel(i+5, 0);
    strip.show();
    strip2.show();
    soundDelay(timeDelay);
  }
}

void pulse(uint32_t col, int key, int timeDelay) {
  while(!colorKeyUp(key)) {
    fadeIn(col, timeDelay);
    fadeOut(col, timeDelay);
  }
  strip.setBrightness(150);
  strip2.setBrightness(150);
}

void pulseOut(uint32_t col, int timeDelay) {
  uint8_t totalNeo = strip.numPixels() + strip2.numPixels();
  uint8_t wheelInc = int(256.0/totalNeo);
  for(int i = 0; i < 4; i++) {
    fadeInRainbow(timeDelay);
    fadeOutRainbow(timeDelay);
    timeDelay+=5;
  }
  strip.setBrightness(0);
  strip2.setBrightness(0);
  strip.show();
  strip2.show();
  delay(1500);
}

void bounce(uint32_t col, uint8_t key, uint8_t timeDelay) {
  setAllPixels(0);
  while(!colorKeyUp(key)) {
    for(int i = 17; i >= 0; i--) {
      if(colorKeyUp(key)) return;
      for(int j=0; j<5; j++) {
        setNeopixel(i+j, col);
      }
      setNeopixel(i+5,0);
      strip.show();
      strip2.show();
      soundDelay(timeDelay);
    }
    for(int i = 0; i <18; i++) {
      if(colorKeyUp(key)) return;
      setNeopixel(i-1, 0);
      for(int j=0; j<5; j++) {
        setNeopixel(i+j, col);
      }
      strip.show();
      strip2.show();
      soundDelay(timeDelay);
    }
  }
}

void rainbowBounce(int timeDelay) {
  setAllPixels(0);
  while(!colorKeyUp(4)) {
    for(int i = 17; i >= 0; i--) {
      setNeopixel(i, Wheel(240));
      setNeopixel(i+1, Wheel(200));
      setNeopixel(i+2, Wheel(150));
      setNeopixel(i+3, Wheel(100));
      setNeopixel(i+4, Wheel(50));
      setNeopixel(i+5, 0);
      strip.show();
      strip2.show();
      soundDelay(timeDelay);
      if(colorKeyUp(4)) return;
    }
    for(int i = 0; i <16; i++) {
      setNeopixel(i, Wheel(240));
      setNeopixel(i+1, Wheel(200));
      setNeopixel(i+2, Wheel(150));
      setNeopixel(i+3, Wheel(100));
      setNeopixel(i+4, Wheel(50));
      setNeopixel(i-1, 0);
      strip.show();
      strip2.show();
      long t = millis();
      soundDelay(timeDelay);
      if(colorKeyUp(4)) return;
    }
  }
}

void neopixelsNext(int i) {
  if(currentPressed == i) setAllPixels(Wheel(currentWheelPos));
  else {
    currentWheelPos += 13;
    setAllPixels(Wheel(currentWheelPos));
  }
  currentPressed = i;
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t key, uint8_t timeDelay) {
  int n;
  if(strip.numPixels() > strip2.numPixels()) n = strip.numPixels();
  else n = strip2.numPixels();
  for(uint16_t i=0; i<n; i++) {
    strip.setPixelColor(i, c);
    strip2.setPixelColor(i, c);
    strip.show();
    strip2.show();
    soundDelay(timeDelay);
  }
}

void fadeIn(uint32_t col, int timeDelay) {
  for(int i = 0; i < 200; i+=15) {
    setAllPixels(col);
    strip.setBrightness(i);
    strip2.setBrightness(i);
    strip.show();
    strip2.show();
    soundDelay(timeDelay);
  }
}

void fadeInRainbow(int timeDelay) {
  int totalNeo = strip.numPixels() + strip2.numPixels();
  int wheelInc = int(256.0/totalNeo);
  for(int i = 0; i < 200; i+=15) {
    for(int i = 0; i < totalNeo-1; i++) setNeopixel(i, Wheel(wheelInc*i));
    strip.setBrightness(i);
    strip2.setBrightness(i);
    strip.show();
    strip2.show();
    soundDelay(timeDelay);
  }
}

void fadeOut(uint32_t col, int timeDelay) {
  for(int i = 200; i >= 0; i-=15) {
    setAllPixels(col);
    strip.setBrightness(i);
    strip2.setBrightness(i);
    strip.show();
    strip2.show();
    soundDelay(timeDelay);
  }
  strip.setBrightness(0);
  strip2.setBrightness(0);
  strip.show();
  strip2.show();
  soundDelay(timeDelay);
}

void fadeOutRainbow(int timeDelay) {
  int totalNeo = strip.numPixels() + strip2.numPixels();
  int wheelInc = int(256.0/totalNeo);
  for(int i = 200; i >= 0; i-=15) {
    for(int i = 0; i < totalNeo-1; i++) setNeopixel(i, Wheel(wheelInc*i));
    strip.setBrightness(i);
    strip2.setBrightness(i);
    strip.show();
    strip2.show();
    soundDelay(timeDelay);
  }
  strip.setBrightness(0);
  strip2.setBrightness(0);
  strip.show();
  strip2.show();
  soundDelay(timeDelay);
}

void rainbow(uint8_t timeDelay) {
  uint16_t i, j;
  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    for(i=0; i<strip2.numPixels(); i++) {
      strip2.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    strip2.show();
    soundDelay(timeDelay);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t timeDelay) {
  uint16_t i, j;
  //for(j=0; j<256*2; j++) { // 5 cycles of all colors on wheel
  while(!colorKeyUp(5)) {
    for(j=0; j<256; j++) { 
      for(i=0; i< strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
      }
      for(i=0; i< strip2.numPixels(); i++) {
        strip2.setPixelColor(i, Wheel(((i * 256 / strip2.numPixels()) + j) & 255));
      }
      strip.show();
      strip2.show();
      soundDelay(timeDelay);
      if(colorKeyUp(5)) return;
    }
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycleOrig(uint8_t timeDelay) {
  uint16_t i, j;
  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
      for(i=0; i< strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
      }
      for(i=0; i< strip2.numPixels(); i++) {
        strip2.setPixelColor(i, Wheel(((i * 256 / strip2.numPixels()) + j) & 255));
      }
      strip.show();
      strip2.show();
      delay(timeDelay);
    }
  
}

void randomLights(uint8_t key, uint8_t timeDelay) {
  uint8_t startval, endval; 
  startval = previousStart + 96;
  endval = startval + 50;
  startval= (key-2)*64;
  endval = startval + 50;
  
  int cycle = 0;
  while(!colorKeyUp(key)) {
    uint8_t randPix = random(strip.numPixels() + strip2.numPixels());
    uint32_t randColor = Wheel((random(startval, endval))%255);
    uint8_t randBrightness;
    if(cycle%4 == 1) randBrightness = 0;
    else if (cycle%4 == 2) randBrightness = random(5,15);
    else if (cycle%4 == 3) randBrightness = random(50,100);
    else randBrightness = random(100,150);
    //setNeopixel(randPix, randColor);
    setPixelBrightness(randPix,randColor, randBrightness);
    strip.show();
    strip2.show();
    soundDelay(timeDelay);
    cycle++;
  }
  previousStart = startval;
}

//Theatre-style crawling lights with 2 colors
void theaterChaseDouble(uint8_t k, uint8_t timeDelay) {
  while(!colorKeyUp(k)) {
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        //if(i%2 == 0) strip.setPixelColor(i+q, noteColors[k]); 
        //else strip.setPixelColor(i+q, noteColors[(k+1)%4]); 
        strip.setPixelColor(i+q, noteColors[k]); 
      }
      for (int i=0; i < strip2.numPixels(); i=i+3) {
        //if(i%2 == 0) strip2.setPixelColor(i+q, noteColors[k]); 
        //else strip2.setPixelColor(i+q, noteColors[(k+1)%4]); 
        strip2.setPixelColor(i+q, noteColors[k]); 
      }
      strip.show();
      strip2.show(); 
      soundDelay(timeDelay);
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
      for (int i=0; i < strip2.numPixels(); i=i+3) {
        strip2.setPixelColor(i+q, 0);        
      }
    }
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t timeDelay) {
  for (int j=0; j<10; j++) {  //do 5 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      for (int i=0; i < strip2.numPixels(); i=i+3) {
        strip2.setPixelColor(i+q, c);
      }
      strip.show();
      strip2.show(); 
      soundDelay(timeDelay);
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
      for (int i=0; i < strip2.numPixels(); i=i+3) {
        strip2.setPixelColor(i+q, 0);        
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t timeDelay) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      for (int i=0; i < strip2.numPixels(); i=i+3) {
        strip2.setPixelColor(i+q, Wheel( (i+j) % 255));
      }
      strip.show();
      strip2.show();
      soundDelay(timeDelay);
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip2.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

void randomBrightRamp(uint8_t k, uint8_t timeDelay) {
  uint8_t b[] = {10, 30, 80, 150};
  strip.setBrightness(b[k]);
  strip2.setBrightness(b[k]);
  for(int i = 0; i < strip.numPixels() + strip2.numPixels(); i++) {
    uint32_t randColor = Wheel(random(0,255));
    setNeopixel(i, randColor);
  }
  strip.show();
  strip2.show();
  while(!colorKeyUp(k)) {
    uint32_t randColor = Wheel(random(0, 255));
    uint8_t pix = random(0, strip.numPixels() + strip2.numPixels());
    //setNeopixel(pix, randColor);
    strip.show();
    strip2.show();
    soundDelay(timeDelay);
  }
  strip.setBrightness(150);
  strip2.setBrightness(150);
}

void blocks(uint8_t k) {
  for(int i = 0; i < 22; i++) {
    if (k == 0) setNeopixel(i, noteColors[0]);
    else if (k == 1) {
      if(i < 9) setNeopixel(i, noteColors[0]);
      else setNeopixel(i, noteColors[1]);
    }
    else if (k == 2) {
      if(i < 4) setNeopixel(i, noteColors[0]);
      else if (i < 9) setNeopixel(i, noteColors[1]);
      else if (i < 15) setNeopixel(i, noteColors[2]);
      else setNeopixel(i, noteColors[3]);
    }
    else {
      setNeopixel(i, noteColors[(i/3)%4]);
    }
  }
  strip.show();
  strip2.show();
}

void set4Blocks() {
  for(int i = 0; i < 22; i++) {
    setNeopixel(i, noteColors[i/5]);
  }
}

void set48locks() {
  for(int i = 0; i < 22; i++) {
    setNeopixel(i, noteColors[i/5]);
  }
}
  
void fade(uint32_t col1, uint32_t col2, uint8_t timeDelay) {
  double h1, h2;
  double l = .5;
  double s = 1;
  double hsl[3];
  rgb2HSL(byte(col1 >> 16), byte(col1 >> 8), byte(col1), hsl);
  h1 = hsl[0];
  s = hsl[1];
  l = hsl[2];
  Serial.print("HSL h1: "); Serial.println(h1);
  rgb2HSL(byte(col2 >> 16), byte(col2 >> 8), byte(col2), hsl);
  h2 = hsl[0];
  Serial.print("HSL h2: "); Serial.println(h2);
  if(h2 > h1) {
    float inc = (h2 - h1)/10.0;
    while (h2 > h1) {
      h1+=inc;
      //Serial.print("h1: "); Serial.println(h1);
      int rgb[3];
      hsl2RGB(h1, s, l, rgb);
      setAllPixels(strip.Color(rgb[0], rgb[1], rgb[2]));
      Serial.print(rgb[0]); Serial.print(" ");
      Serial.print(rgb[1]); Serial.print(" ");
      Serial.println(rgb[2]);
      strip.show();
      soundDelay(timeDelay);
    }
  }
  else {
    double inc = (h1 - h2)/10.0;
    Serial.print("Inc: "); Serial.println(inc);
    while (h2 < h1) {
      h1-=inc;
      Serial.print("h-1: "); Serial.println(h1);
      int rgb[3];
      hsl2RGB(h1, s, l, rgb);
      strip.show();
      setAllPixels(strip.Color(rgb[0], rgb[1], rgb[2]));
      delay(timeDelay);
      //Serial.print(rgb[0]); Serial.print(" ");
      //Serial.print(rgb[1]); Serial.print(" ");
      //Serial.println(rgb[2]);
    }
  }
  
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

void colorShift() {
  for(int i = 0; i < 4; i++) {
    notePositions[i] = (notePositions[i]+15)%255;
    noteColors[i] = Wheel(notePositions[i]);
  }
}

void shiftPosForward(uint8_t steps) {
  uint8_t totNeo = strip.numPixels() + strip2.numPixels();
  for(int i = 0; i < steps; i++) {
    uint32_t temp = getNeopixelColor(totNeo-1);
    for(int i = totNeo-1; i > 0; i--) {
      setNeopixel(i,getNeopixelColor(i-1));
    }
    setNeopixel(0,temp);
  }
}



void shiftPosBack(uint8_t steps) {
  uint8_t totNeo = strip.numPixels() + strip2.numPixels();
  for(int i = 0; i < steps; i++) {
    int temp = getNeopixelColor(0);
    for(int i = 0; i < totNeo-1; i++) {
      setNeopixel(i, getNeopixelColor(i+1));
    }
    setNeopixel(totNeo-1,temp);
  }
}

void shiftBright() {
  int temp = noteBright[21];
  for(int i = 21; i > 0; i--) {
    noteBright[i] = noteBright[i-1];
  }
  noteBright[0] = temp;
}
  
void initializeColors() {
  for(int i = 0; i < 4; i++) noteColors[i] = Wheel(notePositions[i]);
}

void rgb2HSL (uint8_t red, uint8_t green, uint8_t blue, double *a){
  double r = red/255.0;
  double g = green/255.0;
  double b = blue/255.0;
  double v;
  double m;
  double vm;
  double r2, g2, b2;
  double h = 0; // default to black
  double s = 0;
  double l = 0;
  
  v = max(r,g);
  v = max(v,b);
  m = min(r,g);
  m = min(m,b);
  l = (m + v) / 2.0;
  if (l <= 0.0) return;

  vm = v - m;
  s = vm;
  if (s > 0.0) s /= (l <= 0.5) ? (v + m ) : (2.0 - v - m);
  else return;

  r2 = (v - r) / vm;
  g2 = (v - g) / vm;
  b2 = (v - b) / vm;
  if (r == v) h = (g == m ? 5.0 + b2 : 1.0 - g2);
  else if (g == v) h = (b == m ? 1.0 + r2 : 3.0 - b2);
  else h = (r == m ? 3.0 + g2 : 5.0 - r2);
  h /= 6.0;
  a[0] = h;
  a[1] = s;
  a[2] = l;
}

void hsl2RGB(float H, float S, float L, int *a) {
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
    if(S == 0) {
      r = L;
      g = L;
      b = L;
    }
    else {
      float temp1 = 0;
      if(L < .50) {
        temp1 = L*(1 + S);
      }
      else {
        temp1 = L + S - (L*S);
      }
      float temp2 = 2*L - temp1;
      float temp3 = 0;
      for(int i = 0 ; i < 3 ; i++) {
        switch(i) {
          case 0: { //red
            temp3 = H + .33333f;
            if(temp3 > 1) temp3 -= 1;
            hsl_Subfunction(r,temp1,temp2,temp3);
	    break;
	  }
	  case 1: { // green
	    temp3 = H;
	    hsl_Subfunction(g,temp1,temp2,temp3);
	    break;
	  }
	  case 2: { // blue
	    temp3 = H - .33333f;
	    if(temp3 < 0)
    	    temp3 += 1;
	    hsl_Subfunction(b,temp1,temp2,temp3);
	    break;
	  }
	  default:{}
	}
    }
  }
  a[0] = (uint8_t)((((float)r)/100)*255);
  a[1] = (uint8_t)((((float)g)/100)*255);
  a[2] = (uint8_t)((((float)b)/100)*255);
  Serial.print("RGB: ");
  Serial.print(a[0]); Serial.print(" ");
  Serial.print(a[1]); Serial.print(" ");
  Serial.print(a[2]); Serial.println(" ");
}

// This is a subfunction of HSLtoRGB
void hsl_Subfunction(uint8_t& c, const float& temp1, const float& temp2, const float& temp3) {
  if((temp3 * 6) < 1) c = (unsigned int)((temp2 + (temp1 - temp2)*6*temp3)*100);
  else {
    if((temp3 * 2) < 1) c = (unsigned int)(temp1*100);
    else {
      if((temp3 * 3) < 2) c = (unsigned int)((temp2 + (temp1 - temp2)*(.66666 - temp3)*6)*100);
      else c = (unsigned int)(temp2*100);
    }
   }
  return;
}

void soundShift() {
  firstSound =! firstSound;
}

void playSound(byte key) {
  if(millis() - soundT > 100) {
    if(firstSound) Serial.write(key);
    else Serial.write(key+3);
    soundT = millis();
  }
}
