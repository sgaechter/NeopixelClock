/**
 * @file        Neopixel_binary_Clock_Test.ino
 * @brief       Test Sketch to check if the Binary Strips are running correct. Therefore you can
 *              input some digits over the serial Terminal and look how the digits are displayed.
 *
 * @history     March 2, 2019
 *
 * @author      Sven Gaechter <entwicklung@allround-service.biz>
 *
 * @References
 *      1)  Helper Sketch to Proyect "Neopixel Clock by Sven Gächter witch is published on Git Hub:
 *          https://github.com/sgaechter/NeopixelClock.git
 *          
 *
 */
#include "Adafruit_NeoPixel.h"

const int BIN_NEOPIXEL_DATA_PIN = D5;       // WeMos digital pin D5
const int BIN_NEO_NUM_PIXELS = 18;              // WeMos LED Strips, 17 leds, devided in 2x 6 leds, 1x5 leds, cascaded
const int BIN_BRIGHTNESS = 96;              // experiment with this

byte arrayHour[] = {17, 16, 15, 14, 13}; //13, 14, 15, 16, 17}; // 
byte arrayMin[] = {6, 7, 8, 9, 10, 11}; // 11, 10, 9, 8, 7, 6}; //
byte arraySec[] = {5, 4, 3, 2, 1, 0}; // 0, 1, 2, 3, 4, 5}; // 

// Array length count for binary Clock
#define nBitsSec sizeof(arraySec)/sizeof(arraySec[0])
#define nBitsMin sizeof(arrayMin)/sizeof(arrayMin[0])
#define nBitsHr sizeof(arrayHour)/sizeof(arrayHour[0])

// create an instance of NeoPixel called 'bin_strip' for Binary Clock
Adafruit_NeoPixel bin_strip = Adafruit_NeoPixel(
                                BIN_NEO_NUM_PIXELS,
                                BIN_NEOPIXEL_DATA_PIN,
                                NEO_GRB + NEO_KHZ800);

//setPixelColor -- set a single pixel in the NeoPixel ring to a specific RGB value
void setPixelColor(Adafruit_NeoPixel & strip, int index, byte red, byte green, byte blue, int brightness = 256)
{
  strip.setPixelColor(index,
                      (((int)red) * brightness) >> 8,
                      (((int)green) * brightness) >> 8,
                      (((int)blue) * brightness) >> 8
                     );
}

/**
      drawing the pixels for BinaryClock
*/
void dispBinaryHour(byte In){
  for (int i = 0; i <= nBitsHr; i++){ // count from most right Bit (0) to 4th Bit (3){
     if (bitRead(In, i) == 1){
      setPixelColor(bin_strip, arrayHour[i], 35, 3, 196, BIN_BRIGHTNESS / 3);
      Serial.print("Hour array LED: ");
      Serial.println(arrayHour[i]);
    }
  }
}

void dispBinaryMin(byte In){
  for (int i = 0; i <= nBitsMin; i++){ // count from most right Bit (0) to 4th Bit (3){
    if (bitRead(In, i) == 1){
      setPixelColor(bin_strip, arrayMin[i],35, 3, 196, BIN_BRIGHTNESS / 3);
      Serial.print("Hour array LED: ");
      Serial.println(arrayMin[i]);
    }
  }
}

void dispBinarySec(byte In){
  for (int i = 0; i <= nBitsSec; i++){ // count from most right Bit (0) to 4th Bit (3){
    if (bitRead(In, i) == 1){
      setPixelColor(bin_strip, arraySec[i], 35, 3, 196, BIN_BRIGHTNESS / 3);
      Serial.print("Hour array LED: ");
      Serial.println(arraySec[i]);
    }
  }
}


void refreshClock(int hour, int minute, int second) {
  bin_strip.clear();
  dispBinarySec(second);
  dispBinaryMin(minute);
  dispBinaryHour(hour);
  bin_strip.show();
}

void setup() {

  bin_strip.begin();
  bin_strip.clear();
  bin_strip.show(); // Initialize all pixels to 'off'

  Serial.begin(115200);
  Serial.println();
  Serial.println();
}

void loop() {
int incoming;
char recievedChar;
int value;
  // send data only when you receive data:
  if (Serial.available() > 0){
    incoming = Serial.parseInt();
    Serial.print("received: ");
    Serial.println(incoming);
    value = incoming;
    Serial.print("Value: ");
    Serial.println(value);
    refreshClock(value, value, value);
    delay(6000);
    }
  
    
}
