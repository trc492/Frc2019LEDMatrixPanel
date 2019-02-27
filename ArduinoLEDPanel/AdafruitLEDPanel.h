/*
 * Copyright (c) 2017 Titan Robotics Club (http://www.titanrobotics.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <RGBmatrixPanel.h>

#define NUMBER_OF_SLICES    8
#define NUMBER_OF_LINES     32

#define SLICE_SIZE          16
#define SLICE_DIRECTION     2

#define PANEL_WIDTH         64
#define PANEL_HEIGHT        32

//
// The smallest font is 6x8.
//
#define MIN_FONT_WIDTH      6
#define MIN_FONT_HEIGHT     8

//
// Common color values. Can also call the color method specifying separate R, G and B values.
//
#define LED_BLACK           ((unit16_t)0)

#define LED_RED_VERYLOW     ((uint16_t)(3 <<  11))
#define LED_RED_LOW         ((uint16_t)(7 <<  11))
#define LED_RED_MEDIUM      ((uint16_t)(15 << 11))
#define LED_RED_HIGH        ((uint16_t)(31 << 11))

#define LED_GREEN_VERYLOW   ((uint16_t)(7 <<  5))
#define LED_GREEN_LOW       ((uint16_t)(15 << 5))
#define LED_GREEN_MEDIUM    ((uint16_t)(31 << 5))
#define LED_GREEN_HIGH      ((uint16_t)(63 << 5))

#define LED_BLUE_VERYLOW    ((uint16_t)3)
#define LED_BLUE_LOW        ((uint16_t)7)
#define LED_BLUE_MEDIUM     ((uint16_t)15)
#define LED_BLUE_HIGH       ((uint16_t)31)

#define LED_YELLOW_VERYLOW  ((uint16_t)(LED_RED_VERYLOW + LED_GREEN_VERYLOW))
#define LED_YELLOW_LOW      ((uint16_t)(LED_RED_LOW     + LED_GREEN_LOW))
#define LED_YELLOW_MEDIUM   ((uint16_t)(LED_RED_MEDIUM  + LED_GREEN_MEDIUM))
#define LED_YELLOW_HIGH     ((uint16_t)(LED_RED_HIGH    + LED_GREEN_HIGH))

#define LED_MAGENTA_VERYLOW ((uint16_t)(LED_RED_VERYLOW + LED_BLUE_VERYLOW))
#define LED_MAGENTA_LOW     ((uint16_t)(LED_RED_LOW     + LED_BLUE_LOW))
#define LED_MAGENTA_MEDIUM  ((uint16_t)(LED_RED_MEDIUM  + LED_BLUE_MEDIUM))
#define LED_MAGENTA_HIGH    ((uint16_t)(LED_RED_HIGH    + LED_BLUE_HIGH))

#define LED_CYAN_VERYLOW    ((uint16_t)(LED_GREEN_VERYLOW + LED_BLUE_VERYLOW))
#define LED_CYAN_LOW        ((uint16_t)(LED_GREEN_LOW     + LED_BLUE_LOW))
#define LED_CYAN_MEDIUM     ((uint16_t)(LED_GREEN_MEDIUM  + LED_BLUE_MEDIUM))
#define LED_CYAN_HIGH       ((uint16_t)(LED_GREEN_HIGH    + LED_BLUE_HIGH))

#define LED_WHITE_VERYLOW   ((uint16_t)(LED_RED_VERYLOW + LED_GREEN_VERYLOW + LED_BLUE_VERYLOW))
#define LED_WHITE_LOW       ((uint16_t)(LED_RED_LOW     + LED_GREEN_LOW     + LED_BLUE_LOW))
#define LED_WHITE_MEDIUM    ((uint16_t)(LED_RED_MEDIUM  + LED_GREEN_MEDIUM  + LED_BLUE_MEDIUM))
#define LED_WHITE_HIGH      ((uint16_t)(LED_RED_HIGH    + LED_GREEN_HIGH    + LED_BLUE_HIGH))

/**
 * Defining the size of each image slice.
 * This number is calculated using (WIDTH * HEIGHT * 3) / NUMBER_OF_SLICES
 * 3 is included because there are three color channels in RGB colors.
 */

/**
 * This macro clears the entire panel by filling it with BLACK.
 */
#define clearScreen() fillScreen(0)

/**
 * This class extends the RGBmatrixPanel class and provides additional methods to make displaying scrolling-;
 * text very simple.
 */
class AdafruitLEDPanel: RGBmatrixPanel
{
private:

  /**
   * This structure declares the properties associated with a line of text.
   *
   * @param text specifies the text string, null if the line is not used.
   * @param x specifies the x-coordinate of the upper left corner of the text rectangle.
   * @param y specifies the y-coordinate of the upper left corner of the text rectangle.
   * @param fontColor specifies the font color to display the text.
   * @param rotation specifies the orientation of the text displayed.
   * @param fontSize specifies the font size.
   * @param scrollInc specifies whether the text will scroll and if so which direction and how fast.
   * @param cursorX specifies the x-coordinate of the beginning of text. This value is changing while
   *                text is scrolling.
   * @param textWidth specifies the pixel width of the text string.
   */
  typedef struct line_s
  {
    const char *text;
    int16_t x, y;
    uint16_t fontColor;
    uint8_t rotation, fontSize;
    int16_t scrollInc, cursorX;
    uint16_t textWidth;
  } Line;

  int imageSliceIndex;
  int imageBufferIndex;
  int m_rows, m_cols, m_linesArraySize;
  Line *m_lines = NULL;
  char *m_imageBuffer;

  /**
   * This method performs common initialization for all constructors. It is responsible for allowing the
   * lines array and initializing the matrix panel.
   *
   * @param rows specifies the number of rows of the panel.
   * @param cols specifies the number of columns of the panel.
   * @param linesArraySize specifies how many Line structure to be allocated in the lines array.
   */
  void init(int rows, int cols, int linesArraySize)
  {
    imageSliceIndex = 0;
    imageBufferIndex = 0;
    m_rows = rows;
    m_cols = cols;
    m_linesArraySize = linesArraySize;
    m_lines = new Line[linesArraySize];
    memset(m_lines, 0, sizeof(Line)*linesArraySize);
    begin();
    setTextWrap(false);
    clearScreen();
  } //init

public:

  /**
   * Constructors: Creates an instants of the 32-row panel.
   *
   * @param pinA specifies the arduino pin connecting A.
   * @param pinB specifies the arduino pin connecting B.
   * @param pinC specifies the arduino pin connecting C.
   * @param pinD specifies the arduino pin connecting D.
   * @param sclkPin specifies the arduino pin connecting SCLK.
   * @param latchPin specifies the arduino pin connecting LATCH.
   * @param oePin specifies the arduino pin connecting OE.
   * @param doubleBuffered specifies true to use double buffering for smoother animation, false otherwise.
   * @param cols specifies the number of columns of the panel.
   * @param linesArraySize specifies maximum number of lines to be allocated for the lines array.
   */
  AdafruitLEDPanel(
    uint8_t pinA, uint8_t pinB, uint8_t pinC, uint8_t pinD,
    uint8_t sclkPin, uint8_t latchPin, uint8_t oePin,
    boolean doubleBuffered, int cols, int linesArraySize):
    RGBmatrixPanel(pinA, pinB, pinC, pinD, sclkPin, latchPin, oePin, doubleBuffered, cols)
  {
    init(32, cols, linesArraySize);
  } //AdafruitLEDPanel

  /**
   * Constructors: Creates an instants of the 16-row panel.
   *
   * @param pinA specifies the arduino pin connecting A.
   * @param pinB specifies the arduino pin connecting B.
   * @param pinC specifies the arduino pin connecting C.
   * @param sclkPin specifies the arduino pin connecting SCLK.
   * @param latchPin specifies the arduino pin connecting LATCH.
   * @param oePin specifies the arduino pin connecting OE.
   * @param doubleBuffered specifies true to use double buffering for smoother animation, false otherwise.
   * @param cols specifies the number of columns of the panel.
   * @param linesArraySize specifies maximum number of lines to be allocated for the lines array.
   */
  AdafruitLEDPanel(
    uint8_t pinA, uint8_t pinB, uint8_t pinC,
    uint8_t sclkPin, uint8_t latchPin, uint8_t oePin,
    boolean doubleBuffered, int cols, int linesArraySize):
    RGBmatrixPanel(pinA, pinB, pinC, sclkPin, latchPin, oePin, doubleBuffered, cols)
  {
    init(16, cols, linesArraySize);
  } //AdafruitLEDPanel

  /**
   * Destructor: Destroy the object instance by deallocating resources owned.
   */
  ~AdafruitLEDPanel(void)
  {
    delete m_lines;
    m_lines = NULL;
  } //~AdafruitLEDPanel

  /**
   * This method clears the specified line in the lines array.
   *
   * @param index specifies the line index of the array.
   * @return true if successful, false if failed (e.g. index out-of-bound).
   */
  bool clearTextLine(int index)
  {
    bool success = false;
    //
    // Check index within bound.
    //
    if (index < m_linesArraySize)
    {
      if (m_lines[index].text != NULL) delete m_lines[index].text;
      memset((byte *)&m_lines[index], 0, sizeof(Line));
      success = true;
    }

    return success;
  } //clearTextLine

  /**
   * This method erases the specified line on the display.
   *
   * @param index specifies the line index of the array.
   * @return true if successful, false if failed (e.g. index out-of-bound).
   */
  bool eraseTextLine(int index)
  {
    bool success = false;
    //
    // Check index within bound.
    //
    if (index < m_linesArraySize)
    {
      if (m_lines[index].text != NULL)
      {
        setRotation(m_lines[index].rotation);
        //
        // Erase line displayed previously.
        //
        int16_t width = m_lines[index].rotation == 0 || m_lines[index].rotation == 2? m_cols: m_rows;
        fillRect(0, m_lines[index].y, width, m_lines[index].fontSize*MIN_FONT_HEIGHT, 0);
      }
      success = true;
    }

    return success;
  } //eraseTextLine

  /**
   * This method sets the specified line in the lines array with all the info for displaying text on the panel.
   * Note that the (x, y) coordinates is rotation sensitive. If rotation is 0, the text orientation is normal
   * horizontal and (0, 0) corresponds to the upper left corner of the physical panel. If rotation is 2, the
   * text orientation is inverted horizontal and (0, 0) corresponds to the lower right corner of the physica
   * panel.
   *
   * @param index specifies the line index of the array.
   * @param text specifies the text string to be displayed.
   * @param x specifies the x coordinate of the upper left corner of the text rectangle.
   * @param y specifies the y coordinate of the upper left corner of the text rectangle.
   * @param fontColor specifies the font color for displaying the text.
   * @param rotation specifies the text orientation (0: normal horizontal, 1: clockwise vertical, 
   *                 2: inverted horizontal, 3: anti-clockwise vertical).
   * @param fontSize specifies the size of the font (1: 6x8, 2: 12x16, 3: 18x24, 4: 24x32).
   * @param scrollInc specifies the scroll increment (0: no scroll, 1: scroll to the right, -1: scroll to the left).
   * @return true if successful, false if failed (e.g. index out-of-bound).
   */
  bool setTextLine(
    int index,
    const char *text,
    int16_t x, int16_t y,
    uint16_t fontColor,
    uint8_t rotation = 0,
    uint8_t fontSize = 1,
    int16_t scrollInc = 0)
  {
    bool success = false;
    //
    // Check index within bound.
    //
    if (index < m_linesArraySize)
    {
      //
      // Free the previous text if any before we clobber the data.
      //
      clearTextLine(index);

      m_lines[index].text = strdup(text);
      m_lines[index].x = x;
      m_lines[index].y = y;
      m_lines[index].fontColor = fontColor;
      m_lines[index].rotation = rotation;
      m_lines[index].fontSize = fontSize;
      m_lines[index].scrollInc = scrollInc;
      m_lines[index].cursorX = x;
      m_lines[index].textWidth = ((uint16_t)(strlen(text)*fontSize*MIN_FONT_WIDTH));
      success = true;
    }

    return success;
  } //setTextLine

  /**
   * This method is called to display the specified line on the panel by first erasing the line previous displayed,
   * calculating the new cursor position if scrolling is enabled and displaying the text at the new position.
   *
   * @param index specifies the index of the lines array.
   * @param eraseLine specifies true to erase line before display text, false otherwise.
   */
  void displayTextLine(int index, bool eraseLine)
  {
    Serial.println("Doing stuff with screen");
    //
    // Check if index is valid and we have text at that index.
    //
    if (index < m_linesArraySize && m_lines[index].text != NULL)
    {
      //
      // Erase previous displayed line.
      //
      if (eraseLine)
      {
        eraseTextLine(index);
      }

      if (m_lines[index].scrollInc != 0)
      {
        //
        // Scrolling is enabled. Calculate the new cursor position to display text.
        //
        m_lines[index].cursorX += m_lines[index].scrollInc;
        //
        // The following code checks if scrolling passes the end and wraps around to the beginning.
        //
        switch (m_lines[index].rotation)
        {
          case 0:
          case 2:
            if (m_lines[index].scrollInc < 0 && m_lines[index].cursorX + m_lines[index].textWidth <= 0)
            {
              //
              // We are scrolling left and end-of-text passes the left edge of the panel.
              //
              m_lines[index].cursorX = m_cols;
            }
            else if (m_lines[index].scrollInc > 0 && m_lines[index].cursorX >= m_cols)
            {
              //
              // We are scrolling right and beginning-of-text passes the right edge of the panel.
              //
              m_lines[index].cursorX = -m_lines[index].textWidth;
            }
            break;
  
          case 1:
          case 3:
            if (m_lines[index].scrollInc < 0 && m_lines[index].cursorX + m_lines[index].textWidth <= 0)
            {
              //
              // We are scrolling up and end-of-text passes the top edge of the panel.
              //
              m_lines[index].cursorX = m_rows;
            }
            else if (m_lines[index].scrollInc > 0 && m_lines[index].cursorX >= m_rows)
            {
              //
              // We are scrolling down and the beginning-of-text passes the bottom edge of the panel.
              //
              m_lines[index].cursorX = -m_lines[index].textWidth;
            }
            break;
        }
      }
      //
      // Set the new cursor position, text color and size and draw the text on the panel.
      //
      setRotation(m_lines[index].rotation);
      setTextSize(m_lines[index].fontSize);
      setTextColor(m_lines[index].fontColor);
      setCursor(m_lines[index].cursorX, m_lines[index].y);
      print(m_lines[index].text);
      return;
    }
  } //displayTextLine

  /**
   * This method should be called in the loop method to display/scroll text on the panel.
   *
   * @param eraseScreen specifies true to erase screen before refreshing text, false otherwise.
   */
  void displayTextTask(bool eraseScreen)
  {
    if (eraseScreen)
    {
      Serial.println("Clearing slice...");
      clearScreenSlice();
    }
    //
    // Go through each line in the array and display text if any.
    //
    for (int i = 0; i < m_linesArraySize; i++)
    {
      displayTextLine(i, !eraseScreen);
    }
    //
    // Show the buffer we just modified.
    //
    swapBuffers(false);
  } //displayTextTask

  /**
   * This method returns the pixel width of the text with the given line index.
   *
   * @param index specifies the index of the lines array.
   * @return pixel width of the text string.
   */
  int getTextWidth(int index)
  {
    int width = 0;
    //
    // Check if index is within bound and the line is initialized with text.
    //
    if (index < m_linesArraySize && m_lines[index].text != NULL)
    {
      width = m_lines[index].textWidth;
    }

    return width;
  } //getTextWidth

  int setImageSlice(char *imgBuffer)
  {
    int printed = 0;
    if (imageSliceIndex < NUMBER_OF_SLICES)
    {
      int displacement = PANEL_HEIGHT / NUMBER_OF_SLICES;
      
      for (int row = 0; row < displacement; row++)
      {
        for (int col = 0; col < PANEL_WIDTH; col++)
        {
          drawPixel(col, displacement * imageSliceIndex + row,
                    color(imgBuffer[(row * PANEL_WIDTH + col) * 3],
                          imgBuffer[(row * PANEL_WIDTH + col) * 3 + 1],
                          imgBuffer[(row * PANEL_WIDTH + col) * 3 + 2]));
          printed++;
        }
      }
      
      imageSliceIndex++;
    }
    return printed;
  }

  bool readyToContinue()
  {
    if (imageSliceIndex == NUMBER_OF_SLICES)
    {
      imageSliceIndex = 0;
      imageBufferIndex++;
      return true;
    } else {
      return false;
    }
  }

  bool doneRequesting()
  {
    return false;
    if (imageBufferIndex == 2)
    {
      return true;
    } else {
      return false;
    }
  }

  void clearScreenSlice()
  {
    int cleared = 0;
    if (SLICE_DIRECTION == 1)
    {
      Serial.println("clearScreenSlice()");
      for (int row = 0; row < PANEL_HEIGHT; row++)
      {
        for (int col = 0; col < SLICE_SIZE; col++)
        {
          //drawPixel(col + (PANEL_WIDTH - SLICE_SIZE), row, color(29, 233, 182));
          drawPixel(col + (PANEL_WIDTH - SLICE_SIZE), row, color(0,0,0));
          cleared++;
        }
      }
    }
    else
    {
      for (int row = 0; row < PANEL_HEIGHT; row++)
      {
        for (int col = 0; col < PANEL_WIDTH; col++)
        {
          drawPixel(col, row, color(0, 0, 0));
        }
      }
    }
    Serial.println(cleared);
  }

  void displayBuffer()
  {
    swapBuffers(false);
  }

  /**
   * This method returns the color value with the specified RED, GREEN and BLUE values.
   * The RGB values are 8-bit values (i.e. 0 to 255). It will combine the RGB values into
   * a 16-bit value in Color565 format (i.e. RRRRRrrr GGGGGGgg BBBBBbbb => RRRRRGGGGGGBBBBB).
   *
   * @return 16-bit color value.
   */
  uint16_t color(uint8_t r, uint8_t g, uint8_t b)
  {
    return Color888(r, g, b);
  } //color

};  //class AdafruitLEDPanel
