/* 
  audio player graphics helpers

  created 3 July 2026
  modified 4 July 2026
  by foxes
*/

cursorTrack drawWrappedText(String text, int startX, int startY, int marginX2, int textColor, int textSize) {
  cursorTrack ret;
  tft.setTextSize(textSize);
  tft.setTextColor(textColor);

  // Variables to pass to getTextBounds
  int16_t x1, y1;
  uint16_t sampleWidth, sampleHeight;
  
  // Automatically calculate the line height using a benchmark letter
  tft.getTextBounds("M", 0, 0, &x1, &y1, &sampleWidth, &sampleHeight);
  ret.lh = sampleHeight;
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
  ret.y = cursorY;
  return ret;
}

cursorTrack drawPagedWrappedText(String text, int startX, int startY, int marginX2, int textColor, int textSize, int linePos, int maxLines) {
  cursorTrack ret;
  tft.setTextSize(textSize);
  tft.setTextColor(textColor);

  // Variables to pass to getTextBounds
  int16_t x1, y1;
  uint16_t sampleWidth, sampleHeight;
  
  // Automatically calculate the line height using a benchmark letter
  tft.getTextBounds("M", 0, 0, &x1, &y1, &sampleWidth, &sampleHeight);
  ret.lh = sampleHeight;
  ret.y = startY;
  ret.pages = 1;

  int maxWidth = tft.width() - marginX2; // Total width minus right margin
  int lineHeight = sampleHeight + 4; 

  // Need at least 1 text line + 1 marker line
  if (maxLines < 2) {
    maxLines = 2;
  }

  // Reserve the last visible line for "(end)"
  // So text itself gets one fewer line per page
  int textLinesPerPage = maxLines - 1;

  int trailingChars = 6; // space reserved for " (...)"
  int trailingWidth = trailingChars * (6 * textSize);

  if (linePos < 0) {
    linePos = 0;
  }

  // ---------- FIRST PASS: COUNT WRAPPED LINES ----------
  int cursorX = startX;
  int totalLines = 1;

  int wordStart = 0;

  while (wordStart < text.length()) {
    // Skip spaces
    while (wordStart < text.length() && text.charAt(wordStart) == ' ') {
      wordStart++;
    }

    if (wordStart >= text.length()) {
      break;
    }

    int wordEnd = text.indexOf(' ', wordStart);

    if (wordEnd == -1) {
      wordEnd = text.length();
    }

    int wordLength = wordEnd - wordStart;
    int wordWidth = wordLength * (6 * textSize); // Approx width of word in pixels

    int nextWordStart = wordEnd;

    while (nextWordStart < text.length() && text.charAt(nextWordStart) == ' ') {
      nextWordStart++;
    }

    bool hasMoreWords = nextWordStart < text.length();

    // Check if word will fit on current line
    int lineMaxWidth = maxWidth;

    // Last text line of each page gets shortened if there is more text after it
    if (
      hasMoreWords &&
      (totalLines - 1) % textLinesPerPage == textLinesPerPage - 1
    ) {
      lineMaxWidth = maxWidth - trailingWidth;
    }

    if (cursorX + wordWidth > lineMaxWidth) {
      cursorX = startX;
      totalLines++;

      lineMaxWidth = maxWidth;

      if (
        hasMoreWords &&
        (totalLines - 1) % textLinesPerPage == textLinesPerPage - 1
      ) {
        lineMaxWidth = maxWidth - trailingWidth;
      }
    }

    cursorX += wordWidth + (6 * textSize); // Add space width

    wordStart = wordEnd + 1;
  }

  ret.pages = (totalLines + textLinesPerPage - 1) / textLinesPerPage;

  int maxPage = ret.pages - 1;
  linePos = constrain(linePos, 0, maxPage);

  int startLine = linePos * textLinesPerPage;
  int endLine = startLine + textLinesPerPage;

  // ---------- SECOND PASS: DRAW CURRENT PAGE ----------
  cursorX = startX;
  int cursorY = startY;
  int currentLine = 0;
  bool printedAnyWord = false;

  wordStart = 0;

  while (wordStart < text.length()) {
    // Skip spaces
    while (wordStart < text.length() && text.charAt(wordStart) == ' ') {
      wordStart++;
    }

    if (wordStart >= text.length()) {
      break;
    }

    int wordEnd = text.indexOf(' ', wordStart);

    if (wordEnd == -1) {
      wordEnd = text.length();
    }

    String word = text.substring(wordStart, wordEnd);

    int wordWidth = word.length() * (6 * textSize); // Approx width of word in pixels

    int nextWordStart = wordEnd;

    while (nextWordStart < text.length() && text.charAt(nextWordStart) == ' ') {
      nextWordStart++;
    }

    bool hasMoreWords = nextWordStart < text.length();

    // Check if word will fit on current line
    int lineMaxWidth = maxWidth;

    // Last text line of each page gets shortened if there is more text after it
    if (
      hasMoreWords &&
      currentLine % textLinesPerPage == textLinesPerPage - 1
    ) {
      lineMaxWidth = maxWidth - trailingWidth;
    }

    if (cursorX + wordWidth > lineMaxWidth) {
      cursorX = startX; 
      currentLine++;

      lineMaxWidth = maxWidth;

      if (
        hasMoreWords &&
        currentLine % textLinesPerPage == textLinesPerPage - 1
      ) {
        lineMaxWidth = maxWidth - trailingWidth;
      }
    }

    // Once we are past this page, stop drawing
    if (currentLine >= endLine) {
      break;
    }

    // Only print lines inside the current page
    if (currentLine >= startLine) {
      cursorY = startY + ((currentLine - startLine) * lineHeight);

      tft.setCursor(cursorX, cursorY);
      tft.print(word);

      ret.y = cursorY;
      printedAnyWord = true;
    }

    cursorX += wordWidth + (6 * textSize); // Add space width

    wordStart = wordEnd + 1;
  }

  // ---------- PAGE MARKER ----------
  if(linePos != maxPage) {
    if(textColor != SM_BG_C) tft.setTextColor(SM_ELL_TXT_C);
    tft.print(" <...>"); 

  } else {
    if(textColor != SM_BG_C) tft.setTextColor(SM_END_TXT_C);

    int endWidth = 6 * (6 * textSize); // width of " (EOF)"

    if (printedAnyWord && cursorX + endWidth <= maxWidth) {
      tft.print(" <EOF>");
    } else {
      int endY;

      if (printedAnyWord) {
        endY = ret.y + lineHeight;
      } else {
        endY = startY;
      }
      int16_t x2, y2;
      uint16_t w, h;
      tft.setCursor(startX, endY);
      tft.print("<EOF>");

      ret.y = endY;
    }
  }

  return ret;
}

int getPage(int x, int tracksPerPage, int numMenuItems) {
  int maxPageStart = ((numMenuItems - 1) / tracksPerPage) * tracksPerPage;
  return constrain((x / tracksPerPage) * tracksPerPage, 0, maxPageStart);
}
int getItemsOnPage(int page, int tracksPerPage, int numMenuItems) {
  return min(tracksPerPage, numMenuItems - page);
}
