void drawWrappedText(String text, int startX, int startY, int marginX2, int textColor, int textSize) {
  tft.setTextSize(textSize);
  tft.setTextColor(textColor);

  // Variables to pass to getTextBounds
  int16_t x1, y1;
  uint16_t sampleWidth, sampleHeight;
  
  // Automatically calculate the line height using a benchmark letter
  tft.getTextBounds("M", 0, 0, &x1, &y1, &sampleWidth, &sampleHeight);

  int maxWidth = tft.width() - marginX2; // Total width minus right margin
  int cursorX = startX;
  int cursorY = startY;
  int lineHeight = sampleHeight + 4; 

  // Temporary pointers and buffer
  char buffer[200];
  text.toCharArray(buffer, sizeof(buffer));
  
  char* word = strtok(buffer, " "); // Split text by spaces

  while (word != NULL) {
    int wordWidth = strlen(word) * (6 * textSize); // Approx width of word in pixels
    
    // Check if word will fit on current line
    if (cursorX + wordWidth > maxWidth) {
      // Move to next line
      cursorX = startX; 
      cursorY += lineHeight;
      
      // Check if we hit the bottom of the screen
      if (cursorY > tft.height() - lineHeight) {
        break; // Stop if out of screen space
      }
    }

    // Print the word
    tft.setCursor(cursorX, cursorY);
    tft.print(word);

    // Update cursor for the next word
    cursorX += wordWidth + (6 * textSize); // Add space width

    word = strtok(NULL, " "); // Get next word
  }
}

int getPage(int x, int tracksPerPage, int numMenuItems) {
  int maxPageStart = ((numMenuItems - 1) / tracksPerPage) * tracksPerPage;
  return constrain((x / tracksPerPage) * tracksPerPage, 0, maxPageStart);
}
int getItemsOnPage(int page, int tracksPerPage, int numMenuItems) {
  return min(tracksPerPage, numMenuItems - page);
}
