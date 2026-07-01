#include <Encoder.h>
#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>

// Color Definitions
#define BLACK   0x0000
#define WHITE   0xFFFF
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define GRAY    0x5AEB

MCUFRIEND_kbv tft;

// Encoder CLK to 18, DT to 19. SW to 20
Encoder myEnc(18, 19); 
const int buttonPin = 20;
int buttonState = HIGH;

long oldPosition  = -999;
int currentSelection = 0;
int lastSelection = -1;
bool inSubMenu = false;

const int totalItems = 5;
const char* menuItems[totalItems] = {
  "1. Track 1",
  "2. Track 2",
  "3. Track 3",
  "4. Track 4",
  "5. Track 5"
};

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(buttonPin), handleSwitch, RISING);
  
  tft.reset();
  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(0); // Landscape view
  
  drawMainMenu();
}

void loop() {
  if (inSubMenu == false) {
    // Read encoder position and scale down (4 ticks per physical click)
    long newPosition = myEnc.read();
    
    if (newPosition != oldPosition && newPosition % 2 == 0) {
      oldPosition = newPosition;
      
      // Keep selection bounded within 0 to (totalItems - 1)
      currentSelection = newPosition % totalItems;
      if (currentSelection < 0) currentSelection += totalItems;
    }

    // Only redraw if the selection has changed to eliminate flicker
    if (currentSelection != lastSelection) {
      updateMenuHighlight();
      lastSelection = currentSelection;
    } 

    // Check for encoder button press
    if (digitalRead(buttonPin) == LOW) {
      delay(200); // Simple debounce
      executeSelection(currentSelection);
    }
  } else {
    // If inside a submenu, press button again to exit back to main menu
    if (digitalRead(buttonPin) == LOW) {
      delay(200);
      inSubMenu = false;
      lastSelection = -1; // Force a menu redraw
      drawMainMenu();
    }
  } 
}

void drawMainMenu() {
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.setCursor(20, 20);
  tft.println("TRACK LISTING");
  tft.drawFastHLine(20, 55, 280, RED);
}

void updateMenuHighlight() {
  tft.setTextSize(2);
  for (int i = 0; i < totalItems; i++) {
    int yPos = 80 + (i * 35); 
    if (i == currentSelection) {
      // Highlighted item
      tft.fillRect(15, yPos - 5, 290, 28, BLUE);
      tft.setTextColor(WHITE);
    } else {
      // Clear background for non-highlighted items
      tft.fillRect(15, yPos - 5, 290, 28, BLACK);
      tft.setTextColor(WHITE);
    }
    tft.setCursor(25, yPos);
    tft.println(menuItems[i]);
  }
}

void executeSelection(int selection) {
  inSubMenu = true;
  tft.fillScreen(BLACK);
  tft.setTextColor(GREEN);
  tft.setTextSize(3);
  tft.setCursor(20, 40);
  tft.print("Opened:\n  ");
  tft.println(menuItems[selection]);
  
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 180);
  tft.println("Press Click to Return");
}
