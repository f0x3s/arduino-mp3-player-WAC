/* 
  audio player firmware 

  created 2 July 2026
  modified 2 July 2026
  by foxes
*/

#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>
#include <Encoder.h>
#include "SparkFun_MY1690_MP3_Library.h"
#include "testTrackDisplayNames.h"  // #include "trackDisplayNames.h"


// Color Definitions
#define BLACK 0x0000
#define WHITE 0xFFFF
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define GRAY 0x5AEB

// Initialize TFT
MCUFRIEND_kbv tft;

// Encoder & Button Setup
const int CLK_PIN = 18;
const int DT_PIN = 19;
const int BT_PIN = 21;

Encoder knob(CLK_PIN, DT_PIN);

unsigned long lastEncoderTime = 0;
const unsigned long dbEncoderDelay = 5000;  // Encoder Debounce (microseconds)
unsigned long lastButtonTime = 0;
const unsigned long dbButtonDelay = 20000;  // Button Debounce (microseconds)

// knob last position global for checkInputs()
long lastPos = 0;
bool lastButton = false;

// Tracklist located in trackDisplayNames.h
int numMainMenuItems = sizeof(menuItems) / sizeof(menuItems[0]);

// main menu position tracking globals
int randomMainMenuStart;
int mainMenuPos;
int lastMainMenuPos;

struct Request {
  bool button;
  int knob;
};
Request req = { false, 0 };


enum State {
  ERROR,
  MAIN_MENU,
  TRACK,
  INIT,
};

State displayState = INIT;
State lastDisplayState = INIT;

String error = "";

SparkFunMY1690 mp3;

void setup() {
  pinMode(BT_PIN, INPUT_PULLUP);
  randomSeed(analogRead(A0));

  mainMenuPos = random(numMainMenuItems);  // select random track, so not all players start on the same one
  lastMainMenuPos = mainMenuPos;

  tft.reset();
  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(0);

  Serial3.begin(9600);

  if (!mp3.begin(Serial3))  // Begin Player
  {
    error = "mp3 device not detected";
    displayState = ERROR;
  } else {
    int trackCount = mp3.getSongCount();  // Fetch Track Count

    if (trackCount == 0) {
      error = "No tracks found.\n   Make sure the SD card\n   is inserted and there\n   are MP3s on it.";
      displayState = ERROR;
    } else if (trackCount != numMainMenuItems) {
      error = "Track and menu item\n   count mismatch";
      displayState = ERROR;
    }

    mp3.setVolume(30);
    mp3.setPlayModeSingle();
  }


  if (displayState != ERROR) displayState = MAIN_MENU;
  mp3.play();
  //displayState = MAIN_MENU;
}

void loop() {
  req = { false, 0 };
  checkInputs();

  switch (displayState) {
    case (MAIN_MENU):
      if (req.button) {
        // change display state, do nothing with any potential knob input
        // if button and knob trigger simultaneuously, knob motion assumed unintended by user
        // (i.e. knob rotated by act of pressing button)
        displayState = TRACK;
      } else {
        mainMenuPos += req.knob;
        mainMenuPos = constrain(mainMenuPos, 0, numMainMenuItems - 1);

        if (lastDisplayState != displayState)  // moving to main menu from other screen
        {
          // draw everything
          drawMainMenuBG();
          drawMainMenu();
          lastDisplayState = displayState;
        }

        else if (mainMenuPos != lastMainMenuPos)  // change in highlghted menu item
        {
          // update selection
          drawMainMenu();
          lastMainMenuPos = mainMenuPos;
        } else {
          // no change, do nothing
        }
      }

      break;

    case (TRACK):
      if (req.button) {
        // change display state, do nothing with any potential knob input
        // if button and knob trigger simultaneuously, knob motion assumed unintended by user
        // (i.e. knob rotated by act of pressing button)
        displayState = MAIN_MENU;
      } else {
        if (lastDisplayState != displayState)  // moving to sub menu
        {
          // draw everything
          drawSubMenu();
          mp3.playTrackNumber(mainMenuPos);
          lastDisplayState = displayState;
        }
      }
      break;

    case (ERROR):
      if (lastDisplayState != displayState)  // moving to error from other screen
      {
        // draw everything
        drawError();
        lastDisplayState = displayState;
      }
      break;

    default:
      displayState = ERROR;
      lastDisplayState = ERROR;
      error = "unknown display state\n...how did you get here?";
      drawError();
      break;
  }
}

void checkInputs() {
  unsigned long currentTime = micros();

  bool currentButton = (digitalRead(BT_PIN) == LOW);
  long currentPos = knob.read();

  if (currentButton != lastButton) {
    if (currentTime - lastButtonTime > dbButtonDelay) {
      req.button = currentButton;
      lastButtonTime = currentTime;
      lastButton = currentButton;
    }
  }

  if (currentPos != lastPos && currentPos % 4 == 0) {
    if (currentTime - lastEncoderTime > dbEncoderDelay) {
      req.knob = (-1) * constrain(currentPos - lastPos, -1, 1);
      lastPos = currentPos;
      lastEncoderTime = currentTime;
    }
  }
}

void drawMainMenuBG() {
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.setCursor(20, 20);
  tft.println("TRACK LISTING");
  tft.drawFastHLine(20, 55, 280, RED);
}

void drawMainMenu() {
  tft.setTextSize(2);
  for (int i = 0; i < numMainMenuItems; i++) {
    int yPos = 80 + (i * 35);
    if (i == mainMenuPos) {
      tft.fillRect(15, yPos - 5, 290, 28, BLUE);
      tft.setTextColor(WHITE);
    } else {
      tft.fillRect(15, yPos - 5, 290, 28, BLACK);
      tft.setTextColor(WHITE);
    }
    tft.setCursor(25, yPos);
    tft.println(menuItems[i]);
  }
}

void drawSubMenu() {
  tft.fillScreen(BLACK);
  tft.setTextColor(GREEN);
  tft.setTextSize(3);
  tft.setCursor(20, 40);
  tft.print("Opened:\n  ");
  tft.println(menuItems[mainMenuPos]);

  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 180);
  tft.println("Press Knob to Return");
}

void drawError() {
  tft.fillScreen(RED);
  tft.setTextColor(BLACK);
  tft.setTextSize(4);
  tft.setCursor(20, 40);
  tft.print("ERROR");
  tft.setTextSize(2);
  tft.setCursor(20, 80);
  tft.print("We've run into an issue");
  tft.setCursor(20, 120);
  tft.print("Please Alert Nearby");
  tft.setCursor(20, 140);
  tft.print("Gallery Attendant");

  int16_t x1, y1;
  uint16_t w, h;

  tft.getTextBounds(error, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(20, 440 - h);
  tft.print("Log: ");
  tft.setCursor(20, 460 - h);
  tft.print(error);
}
