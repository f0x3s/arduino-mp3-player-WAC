/* 
  audio player firmware 

  created 2 July 2026
  modified 4 July 2026
  by foxes
*/

#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>
#include <Encoder.h>
#include "SparkFun_MY1690_MP3_Library.h"
#include "testTrackDisplayNames.h"  // #include "trackDisplayNames.h"
#include "styleSheet.h"
// Initialize TFT & graphics helpers
MCUFRIEND_kbv tft;
struct cursorTrack {
  int y;
  int lh;
  int pages;
};

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
int maxDescChars = 0;
const int TRACKS_PER_PAGE = 10;
int currentPage;
int lastPage;

// main menu position tracking globals
int randomMainMenuStart;
int mainMenuPos;
int lastMainMenuPos;

int lastLinePos;
int currentLinePos;
int maxPage=100;

struct Request {
  bool button;
  int knob;
};
Request req = { false, 0 };

#include "gfxHelpers.h"

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
const int CD_PIN = 23;

bool lastCard = true;

void setup() {
  pinMode(BT_PIN, INPUT_PULLUP);
  randomSeed(analogRead(A0));

  mainMenuPos = random(numMainMenuItems);  // select random track, so not all players start on the same one
  lastMainMenuPos = mainMenuPos;

  currentPage = getPage(mainMenuPos, TRACKS_PER_PAGE, numMainMenuItems);
  lastPage = getPage(lastMainMenuPos, TRACKS_PER_PAGE, numMainMenuItems);

  tft.reset();
  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(0);

  Serial3.begin(9600);
  pinMode(CD_PIN, INPUT_PULLUP);

  if (!mp3.begin(Serial3))  // Begin Player
  {
    error = "mp3 device not detected";
    displayState = ERROR;
  } else {
    checkTrackCount();

    mp3.setVolume(30);
    mp3.setPlayModeSingle();
  }



  for(int i = 0; i < numMainMenuItems; i++) {
    if (strlen(desc[i]) > maxDescChars) maxDescChars = strlen(desc[i]);
  }


  if (displayState != ERROR) displayState = MAIN_MENU;
  mp3.playTrackNumber(mainMenuPos + 1);
}

void loop() {
  req = { false, 0 };
  checkInputs();
  checkCard();

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
          currentLinePos = 0;
          lastLinePos = 0;
          currentPage = getPage(lastMainMenuPos, TRACKS_PER_PAGE, numMainMenuItems);
          lastPage = getPage(mainMenuPos, TRACKS_PER_PAGE, numMainMenuItems);
          drawMainMenuBG();
          drawMainMenu();
          lastDisplayState = displayState;
        }

        else if (mainMenuPos != lastMainMenuPos)  // change in highlghted menu item
        {
          // update selection
          currentPage = getPage(mainMenuPos, TRACKS_PER_PAGE, numMainMenuItems);
          lastPage = getPage(lastMainMenuPos, TRACKS_PER_PAGE, numMainMenuItems);


          drawMainMenuUpdate();

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

        if (maxPage <= 1) {
          currentLinePos = 0;
          lastLinePos = 0;
        } else {
          currentLinePos += req.knob;
          currentLinePos = constrain(currentLinePos, 0, maxPage - 1);
        }
        if (lastDisplayState != displayState)  // moving to sub menu
        {
          // draw everything
          
          drawSubMenu();

          // send play command if new track
          int currentTrack = mp3.getTrackNumber() - 1;
          if (mainMenuPos != currentTrack) {
            mp3.playTrackNumber(mainMenuPos + 1);
          }

          lastDisplayState = displayState;
        } else if (currentLinePos != lastLinePos) {
          drawSubMenuUpdate();

          lastLinePos = currentLinePos;
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
int checkTrackCount() {
  int trackCount = mp3.getSongCount();  // Fetch Track Count

  if (trackCount == 0) {
    error = "No tracks found. Make sure the SD card is inserted and there are MP3s on it.";
    displayState = ERROR;

  } else if (trackCount != numMainMenuItems) {
    error = "Track and menu item\n   count mismatch";
    displayState = ERROR;
  }
  return trackCount;
}
void checkCard() {
  bool cardInserted = digitalRead(CD_PIN) == LOW;

  if (cardInserted != lastCard) {
    if (!cardInserted) {
      mp3.stopPlaying();
      error = "No SD card inserted.";
      displayState = ERROR;

    } else {
      delay(500);
      mp3.reset();

      int retries = 0;

      while (checkTrackCount() != numMainMenuItems) {
        if (retries >= 25) {
          mp3.reset();
          retries = 0;
        }
        delay(10);
        retries++;
      }


      mp3.setVolume(30);
      mainMenuPos = random(numMainMenuItems);  // select random track, so not all players start on the same one
      lastMainMenuPos = mainMenuPos;

      currentPage = getPage(mainMenuPos, TRACKS_PER_PAGE, numMainMenuItems);
      lastPage = getPage(lastMainMenuPos, TRACKS_PER_PAGE, numMainMenuItems);

      mp3.playTrackNumber(mainMenuPos + 1);

      displayState = MAIN_MENU;
    }
    lastCard = cardInserted;
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

  long delta = currentPos - lastPos;

  if (abs(delta) >= 4) {
    if (currentTime - lastEncoderTime > dbEncoderDelay) {
      req.knob = (-1) * constrain(delta, -1, 1);
      lastPos = currentPos;
      lastEncoderTime = currentTime;
    }
  }
}

void drawMainMenuBG() {
  cursorTrack cP;

  tft.fillScreen(MM_BG_C);

  tft.setTextColor(MM_H_TXT_C);
  tft.setTextSize(3);
  tft.setCursor(MM_MARGIN, 20);
  tft.println("TRACK LISTING");

  tft.drawFastHLine(MM_MARGIN - 5, 55, tft.width() - 2 * (MM_MARGIN - 5), MM_H_HR_C);

  tft.setTextSize(2);
  tft.setCursor(MM_MARGIN, 440);
  tft.setTextColor(MM_PC_C);
  tft.print("Page ");
  tft.print((currentPage / 10) + 1);
  tft.print(" of ");
  tft.print((getPage(numMainMenuItems, TRACKS_PER_PAGE, numMainMenuItems) / 10) + 1);

  tft.setCursor(MM_MARGIN, 460);
  tft.setTextColor(MM_NP_C);
  String nowPlaying = String(menuItems[mainMenuPos]);
  tft.println("NOW PLAYING: " + nowPlaying);
}

void drawMainMenu() {
  tft.setTextSize(2);
  for (int i = 0; i < TRACKS_PER_PAGE; i++) {
    int yPos = 80 + (i * 35);
    int index = currentPage + i;

    if (index >= numMainMenuItems) break;

    if (index == mainMenuPos) {
      tft.fillRect(MM_MARGIN - 5, yPos - 5, tft.width() - 2 * (MM_MARGIN - 5), 28, MM_HL_C);
    }

    tft.setTextColor(MM_TXT_C);
    tft.setCursor(MM_MARGIN + 5, yPos);
    tft.print(index + 1);
    tft.print(". ");
    tft.println(menuItems[index]);
  }
}

void drawMainMenuUpdate() {
  tft.setTextSize(2);
  int yPos;

  if (currentPage != lastPage) {
    int itemsOnLastPage = getItemsOnPage(lastPage, TRACKS_PER_PAGE, numMainMenuItems);

    for (int i = 0; i < itemsOnLastPage; i++) {
      int index = lastPage + i;
      yPos = 80 + (i * 35);
      if (index == lastMainMenuPos) {
        tft.fillRect(MM_MARGIN - 5, yPos - 5, tft.width() - 2 * (MM_MARGIN - 5), 28, MM_BG_C);
      } else {
        tft.setCursor(MM_MARGIN + 5, yPos);
        tft.setTextColor(MM_BG_C);
        tft.print(index + 1);
        tft.print(". ");
        tft.println(menuItems[index]);
      }
    }

    String erase = "Page " + String((lastPage / 10) + 1);
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds("Page ", 0, 0, &x1, &y1, &w, &h);

    tft.setCursor(MM_MARGIN + w, 440);
    tft.setTextColor(MM_BG_C);
    tft.print((lastPage / 10) + 1);

    tft.setCursor(MM_MARGIN, 440);
    tft.setTextColor(MM_PC_C);
    tft.print("Page ");
    tft.print((currentPage / 10) + 1);
    tft.print(" of ");
    tft.print((getPage(numMainMenuItems, TRACKS_PER_PAGE, numMainMenuItems) / 10) + 1);
    drawMainMenu();

  } else {
    yPos = 80 + ((lastMainMenuPos % TRACKS_PER_PAGE) * 35);
    tft.fillRect(MM_MARGIN - 5, yPos - 5, tft.width() - 2 * (MM_MARGIN - 5), 28, MM_BG_C);
    tft.setCursor(MM_MARGIN + 5, yPos);
    tft.setTextColor(MM_TXT_C);
    tft.print(lastMainMenuPos + 1);
    tft.print(". ");
    tft.println(menuItems[lastMainMenuPos]);

    yPos = 80 + ((mainMenuPos % TRACKS_PER_PAGE) * 35);
    tft.fillRect(MM_MARGIN - 5, yPos - 5, tft.width() - 2 * (MM_MARGIN - 5), 28, MM_HL_C);
    tft.setTextColor(MM_TXT_C);
    tft.setCursor(MM_MARGIN + 5, yPos);
    tft.print(mainMenuPos + 1);
    tft.print(". ");
    tft.println(menuItems[mainMenuPos]);
  }
}

cursorTrack subCP;

void drawSubMenu() {
  int16_t x1, y1;
  uint16_t w, h;
  cursorTrack cP;
  tft.fillScreen(SM_BG_C);

  tft.setTextColor(SM_H_TXT_C);
  tft.setTextSize(3);
  tft.setCursor(SM_MARGIN, 40);

  cP = drawWrappedText("Now Playing",
        SM_MARGIN, 2 * SM_MARGIN,
        SM_MARGIN,
        SM_H_TXT_C,
        SM_H_TXT_SZ);

  String itemName = String(mainMenuPos + 1) + ". " + String(menuItems[mainMenuPos]);
  cP = drawWrappedText(itemName,
        SM_MARGIN,
        cP.y + (SM_LS * cP.lh),
        SM_MARGIN,
        SM_SH_TXT_C,
        SM_SH_TXT_SZ);

  subCP = cP;

  cP = drawPagedWrappedText(desc[mainMenuPos],
        SM_MARGIN,
        cP.y + (SM_LS * 2 * cP.lh),
        SM_MARGIN,
        SM_DES_TXT_C,
        SM_DES_TXT_SZ,
        currentLinePos,
        MAX_SM_LN);
        
  maxPage = cP.pages;

  if (maxPage <= 1) {
    currentLinePos = 0;
    lastLinePos = 0;
  }
  
  String scrollMessage = "Turn Knob to Scroll";
  String retMessage = "Press Knob to Return";
  tft.setTextSize(SM_RET_TXT_SZ);
  tft.getTextBounds(retMessage, 0, 0, &x1, &y1, &w, &h);

  cP = drawWrappedText(scrollMessage,
        SM_MARGIN,
        (tft.height() - SM_MARGIN) - 2 * h,
        SM_MARGIN,
        SM_RET_TXT_C,
        SM_RET_TXT_SZ);

  cP = drawWrappedText(retMessage,
        SM_MARGIN,
        cP.y + (SM_LS * cP.lh),
        SM_MARGIN,
        SM_RET_TXT_C,
        SM_RET_TXT_SZ);
}

void drawSubMenuUpdate() {
  cursorTrack cP;
  cP = drawPagedWrappedText(desc[mainMenuPos],
        SM_MARGIN,
        subCP.y + (SM_LS * 2 * subCP.lh),
        SM_MARGIN,
        SM_BG_C,
        SM_DES_TXT_SZ,
        lastLinePos,
        MAX_SM_LN);

  cP = drawPagedWrappedText(desc[mainMenuPos],
        SM_MARGIN,
        subCP.y + (SM_LS * 2 * subCP.lh),
        SM_MARGIN,
        SM_DES_TXT_C,
        SM_DES_TXT_SZ,
        currentLinePos,
        MAX_SM_LN);
  
  maxPage = cP.pages;
}
void drawError() {
  int16_t x1, y1;
  uint16_t w, h;
  cursorTrack cP;

  tft.fillScreen(ER_BG_C);
  cP = drawWrappedText("ERROR",
        ER_MARGIN,
        2 * ER_MARGIN,
        ER_MARGIN,
        ER_H_TXT_C,
        ER_H_TXT_SZ);

  cP = drawWrappedText("We've run into an issue",
        ER_MARGIN,
        cP.y + (ER_LS * cP.lh),
        ER_MARGIN,
        ER_SH_TXT_C,
        ER_SH_TXT_SZ);

  cP = drawWrappedText("Please Alert Nearby Gallery Attendant",
        ER_MARGIN,
        cP.y + (ER_LS * 2 * cP.lh),
        ER_MARGIN,
        ER_SH_TXT_C,
        ER_SH_TXT_SZ);

  tft.setTextSize(ER_LG_TXT_SZ);
  tft.getTextBounds(error, 0, 0, &x1, &y1, &w, &h);

  cP = drawWrappedText("Log: ",
        ER_MARGIN,
        (tft.height() - (2 * ER_MARGIN)) - h,
        ER_MARGIN,
        ER_LG_TXT_C,
        ER_LG_TXT_SZ);

  cP = drawWrappedText(error,
        ER_MARGIN,
        (tft.height() - ER_MARGIN) - h,
        ER_MARGIN,
        ER_LG_TXT_C,
        ER_LG_TXT_SZ);
}