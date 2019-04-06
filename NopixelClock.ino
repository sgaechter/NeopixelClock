/**
   @file        NeopixelClock.ino
   @brief       Time display program for an ESP8266 and a single 60-Neopixel Ring

   @history     January 4, 2019

   @author      Sven Gaechter <entwicklung@allround-service.biz>

   @References
        1)  AdaFruit's NeoPixel "Uberguide"
            https://learn.adafruit.com/adafruit-neopixel-uberguide/the-magic-of-neopixels

        2)  NIST explaination of services:
            https://www.nist.gov/pml/time-and-frequency-division/services/internet-time-service-its

        3)  earlier work by Joshua Brooks
            https://www.instructables.com/id/Desktop-NeoPixel-Clock/

        4)  forket from the Project by Allan Schwartz
            http://www.whatimade.today/desktop-neopixel-clock/

        5)  current Sketch is published on Git Hub:
            https://github.com/sgaechter/NeopixelClock.git

*/

#include "Adafruit_NeoPixel.h"
#include <ESP8266WiFi.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <Time.h>

/*---------------------------------------------------------------------
  /   Constants and Global Variables
  /----------------------------------------------------------------------*/
// WIFI
const char* WIFI_SSID = "YOUR_SSID";     // YOUR WiFi SSID here
const char* WIFI_PWD = "YOUR_SECRET_KEY";      // YOUR WiFi password here

// NeoPixel Settings
const int NEOPIXEL_DATA_PIN = D6;       // WeMos digital pin D6
const int NEO_NUM_PIXELS = 60;     // one Ring, 59 pixels
const int BRIGHTNESS = 96;              // experiment with this

/*
  int hourPix = 0;
  int minutePix = 0;
  int secPix = 0;
*/

// create an instance of NeoPixel called 'strip'
Adafruit_NeoPixel strip = Adafruit_NeoPixel(
                            NEO_NUM_PIXELS,
                            NEOPIXEL_DATA_PIN,
                            NEO_GRB + NEO_KHZ800);

// TCP connection
const uint16_t TCP = 13;
const char *NistHostName = "time.nist.gov"; // our NTP server url
IPAddress NistServerAddr(0, 0, 0, 0);       // set via DNA lookup, maybe to 129.6.15.28

// global Time synchonization variables
time_t      clocktime_s = 0;        // seconds on the clock (since midnight)
clock_t     clockset_at_ms = 0;     // milliseconds this program has been running,
// ... at the moment of the clock synchronization event
bool        clock_sync_ok = false;

// global timekeeping variables
int clock_hour, clock_minute, clock_second, clock_milli;

// define the local timezone offset from GMT
const time_t  TIMEZONEOFFSET = (1 * 60 * 60);     // Swizerland is GMT + 1 hours, Summertime + 2 hours

/*
  ---------------------------------------------------------------------
*/


/**
        getTime_from_NIST -- retrieve the current time from nist.time.gov, by
            opening a TCP/IP (port 13) connection and reading the packet
*/
time_t  getTime_from_NIST(void)
{
  // Check WiFi connection status
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("lost WiFi connection");
    return -1;
  }

  Serial.printf("Connecting to: %s: %d\r\n", NistHostName, TCP);
  WiFiClient client;
  if (!client.connect(NistServerAddr, TCP)) {
    Serial.println("Connection failed");
    return -1;
  }

  // send empty request packet through TCP connection.
  client.print("\n");

  // Read the reply packet from server and print this to Serial
  String line = client.readStringUntil('\r');

  // debug printf ... print whatever we received
  Serial.print(line);

  client.stop();   // Close connection
  return parse_NTP_response(line);
}


/**
        parseNTPresponse -- parse the packet from nist.time.gov,
            returning the number of seconds since midnight, GMT
*/
time_t  parse_NTP_response(String line)
{
  if (line && line.length() > 24 ) {

    // now parse this string.  it looks like:
    //  jjjjj mm/dd/yyyy hh:mm:ss ....

    char buf[50];
    line.toCharArray(buf, sizeof buf);

    int blanks[5] = { 0, };  // index of the spaces within buf
    blanks[2] = 0;
    for ( int i = 0, j = 0; (i < strlen(buf)) && (j < 5); i++ )  {
      char c = buf[i];
      // find the first couple of spaces, delimiting the first couple of tokens
      if ( c == ' ' || isspace(c)) {
        blanks[j++] = i;
      }
    }

    // the third token looks like "hh:mm:ss"
    if (blanks[2]) {
      char *tok = &buf[blanks[2] + 1];

      int nist_hour = atoi(&tok[0]);
      int nist_min = atoi(&tok[3]);
      int nist_sec = atoi(&tok[6]);

      // debug printf ... print parsed time on the console
      Serial.println(" hour " + String(nist_hour) +
                     " min " + String(nist_min) +
                     " sec " + String(nist_sec));

      return (nist_hour * 60L * 60L) + (nist_min * 60L) + nist_sec;
    }
  }
  return 0;
}


/**
        sychronizeTime -- called to open a connection to the time server
            and set global variables for the current time
*/
bool sychronizeTime(void)
{
  time_t  nist_time_s = getTime_from_NIST();
  if (nist_time_s < 0) {
    Serial.println("time syncronization failure");
    clock_sync_ok = false;
  }
  else if (nist_time_s == 0) {
    Serial.println("NTP packet parsing failure");
    clock_sync_ok = false;
  }
  else {
    // otherwise, success!
    // remember now_s, and the current millisecond clock
    clocktime_s = nist_time_s + TIMEZONEOFFSET;
    clockset_at_ms = millis();
    Serial.println("time syncronized!");
    clock_sync_ok = true;
  }
  return clock_sync_ok;
}


/**
        setPixelColor -- set a single pixel in the NeoPixel ring to a specific RGB value
*/
void setPixelColor(Adafruit_NeoPixel & strip, int index, byte red, byte green, byte blue, int brightness = 256)
{
  strip.setPixelColor(index,
                      (((int)red) * brightness) >> 8,
                      (((int)green) * brightness) >> 8,
                      (((int)blue) * brightness) >> 8
                     );
}

/*

   drawing the global classification representing the grid of hours around the Clock
*/
void  drawGlobClassification() {
  int hc = NEO_NUM_PIXELS / 12;
  setPixelColor(strip, 0 % 12 * hc, 255, 255, 255, BRIGHTNESS / 6);
  setPixelColor(strip, 1 % 12 * hc, 255, 255, 255, BRIGHTNESS / 6);
  setPixelColor(strip, 2 % 12 * hc, 255, 255, 255, BRIGHTNESS / 6);
  setPixelColor(strip, 3 % 12 * hc, 255, 255, 255, BRIGHTNESS / 6);
  setPixelColor(strip, 4 % 12 * hc, 255, 255, 255, BRIGHTNESS / 6);
  setPixelColor(strip, 5 % 12 * hc, 255, 255, 255, BRIGHTNESS / 6);
  setPixelColor(strip, 6 % 12 * hc, 255, 255, 255, BRIGHTNESS / 6);
  setPixelColor(strip, 7 % 12 * hc, 255, 255, 255, BRIGHTNESS / 6);
  setPixelColor(strip, 8 % 12 * hc, 255, 255, 255, BRIGHTNESS / 6);
  setPixelColor(strip, 9 % 12 * hc, 255, 255, 255, BRIGHTNESS / 6);
  setPixelColor(strip, 10 % 12 * hc, 255, 255, 255, BRIGHTNESS / 6);
  setPixelColor(strip, 11 % 12 * hc, 255, 255, 255, BRIGHTNESS / 6);
}

/**
        drawHourHand -- update the NeoPixel ring, with the pixel representing the hour
*/
void drawHourHand(int hour, int minute) {

  int hourClassification = NEO_NUM_PIXELS / 12;
  int hourminPix = minute / (60 / hourClassification);
  int hourPlus = hour % 12;
  int hourPix = 0;
  if (hourPlus == 0) {
    hourPix = (hour * hourClassification) + hourminPix;
  }
  else {
    hourPix = (hourPlus * hourClassification) + hourminPix;
  }

  if (hourPix == 12) {
    // special case for noon
    int special = 0 + hourminPix;
    setPixelColor(strip, special, 255, 0, 0, BRIGHTNESS / 3);
  }
  else {
    setPixelColor(strip, hourPix, 255, 0, 0, BRIGHTNESS / 3);
  }
}

/**
        drawMinuteHand -- update the  NeoPixel ring, with the pixel representing the minutes
*/
void drawMinuteHand(int minute) {

  int minutePix = minute / (60 / NEO_NUM_PIXELS);     //count of Pixels / 60minutes * count Hour
  setPixelColor(strip, minutePix, 0, 255, 0, BRIGHTNESS);
}

/**
        drawSecondHand -- update the NeoPixel ring, with the pixel representing the seconds
*/
void drawSecondHand(int sec) {
  int secPixClass = 60 / NEO_NUM_PIXELS;
  int secPix = sec / secPixClass;
  setPixelColor(strip, secPix, 255, 195, 0, BRIGHTNESS / 2);
}

/**
        refreshClock -- update both NeoPixel rings, with the current time
*/
void refreshClock(int hour, int minute, int second) {
  strip.clear();
  drawGlobClassification();
  drawHourHand(hour, minute);
  drawMinuteHand(minute);
  drawSecondHand(second);
  strip.show();
}

/**
        setup -- initialization of time display program,
            including initializing the NeoPixels,
            initializing the serial port,
            connecting to WiFi,
*/
void setup() {
  strip.begin();
  strip.clear();
  strip.show(); // Initialize all pixels to 'off'

  Serial.begin(115200);
  Serial.println();
  Serial.println();

  WiFi.begin(WIFI_SSID, WIFI_PWD);

  // try for 2 minutes to connect
  for (int trys = 0; trys < 2 * 60; trys++) {
    if  (WiFi.status() == WL_CONNECTED) {
      Serial.print("\r\nWifi connected: ");
      Serial.println(WiFi.localIP());
      break;
    }
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");

  int rc = WiFi.hostByName(NistHostName, NistServerAddr);
  if (rc < 0)
    Serial.println("GetHostByName returns : " + String(rc));
  else {
    Serial.print("GetHostByName resolved " + String(NistHostName) + ": ");
    Serial.println(NistServerAddr);
  }
}


/**
        timeKeeping -- called once per loop to manage to time variables
*/
void timeKeeping() {
  clock_t  now_ms = millis();
  clock_t  delta_ms = now_ms - clockset_at_ms;       // how many milliseconds since we synced

  time_t   secs_since_midnight = clocktime_s + delta_ms / 1000;

  clock_hour = (secs_since_midnight / 3600L) % 24L;
  clock_minute = (secs_since_midnight / 60L) % 60L;
  clock_second = secs_since_midnight % 60L;
  clock_milli = now_ms % 1000L;

  if (delta_ms > 3600 * 1000) {
    // Once per hour sychronize the time to nist.time.gov
    sychronizeTime();
  }
  if (clock_second == 15 && !clock_sync_ok) {
    // if we initially failed to connect
    // keep trying once per minute to sychronize the time to nist.time.gov
    sychronizeTime();
  }
}


/**
        loop -- time display program for a ESP8266 and a pair of NeoPixel rings
*/
const clock_t  TIMEBUDGET_ms = 1000; // we have a time budget of 1000ms per loop interation

void loop() {
  clock_t  t0_ms = millis();

  timeKeeping();
  refreshClock(clock_hour, clock_minute, clock_second);

  // debug printf ... print time on the console
  char timestring[16];
  sprintf(timestring, "%02d:%02d:%02d.%03d", clock_hour, clock_minute, clock_second, clock_milli);
  Serial.println(timestring);

  // end of loop interation, how long to delay?
  clock_t  time_consumed_ms = millis() - t0_ms;
  if (time_consumed_ms < TIMEBUDGET_ms)
    delay(TIMEBUDGET_ms - time_consumed_ms);
}
