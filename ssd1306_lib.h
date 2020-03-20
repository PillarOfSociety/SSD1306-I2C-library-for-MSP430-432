
#ifndef SSD1306_LIB_H_
#define SSD1306_LIB_H_

#define FILL_Black  0
#define FILL_White  0xFF

/******************************************************************************************************************************************
* void ssd1306Init(void)
* sets up display for use.
* Must be called first
*
* returns nothing
*******************************************************************************************************************************************/
void ssd1306Init(void);
void sendCommand (unsigned char command);
void setCursor (unsigned char x, unsigned char p);
void drawPixel (unsigned char x, unsigned char y, unsigned char clear);

/***********************************************************
 * void drawDot (unsigned char x, unsigned char y)
 * Sets pixel as xy pos white.  Will clear rows above or below for a given line/page
 * TODO DOESNT WORK RIGHT YET
 ***********************************************************/
void drawDot (unsigned char x, unsigned char y);

/******************************************************************************************************************************************
 * void fillDisplay(unsigned char color)
 * Fills display with either black or white pixels
 *
 * 0 for all black (essentially clear screen)
 * FF for all white
 *
 * Returns Nothing
 *****************************************************************************************************************************************/
void fillDisplay(unsigned char color);
void drawImage(unsigned char x, unsigned char y, unsigned char sx,
                       unsigned char sy, const unsigned char img[],
                       unsigned char invert);
void draw6x8Str(unsigned char x, unsigned char p, const char str[],
                        unsigned char invert, unsigned char underline);
void draw12x16Str(unsigned char x, unsigned char y, const char str[],
                          unsigned char invert);


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//new stuff for buffered writes
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int ssd1306_buff_begin(int displayHeight,int displayWidth);
void ssd1306_buff_end(void);
void ssd1306_display(void);


#endif /* SSD1306_LIB_H_ */
