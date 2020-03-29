/*********************************************************************************************
 * Origonal code from here:
 * https://github.com/boykod/SSD1306-I2C-library-for-MSP430-432
 *
 * Fork 3-3-20
 * AJS
 * Rev  2.0
 ********************************************************************************************/

//#include "ti/devices/msp432p4xx/inc/msp.h"
#include <msp430.h> //this should cover all MSP430s
#include <stdio.h>
//#include <stdint.h>
#include <stdlib.h>
#include "address.h"
#include "font6x8.h"
#include "font12x16.h"
#include "ssd1306_lib.h"
#include "ssd1306_i2c_lib.h"    //use MSP430 version


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~GLOBALS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
unsigned char data [2];
unsigned char *dataBuffer;      //used for direct writes //so array persists during interrupt driven TX.

unsigned char * displayBuff_ptr = NULL;    //full buffer
unsigned char* displaybuffer = NULL;  //used for data portion of buffered writes
unsigned int displayBufLen;

int Height,Width;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~FUNCTIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


void ssd1306Init(void) {

    i2c_init(); //setup hardware interface

    sendCommand(SSD1306_DISPLAY_OFF);                                   /* Entire Display OFF */
    sendCommand(SSD1306_SET_DISPLAY_CLOCK_DIV);                         /* Set Display Clock Divide Ratio and Oscillator Frequency */
    sendCommand(0x80);                                                  /* Default Setting for Display Clock Divide Ratio and Oscillator Frequency that is recommended */
    sendCommand(SSD1306_SET_MULTIPLEX_RATIO);                           /* Set Multiplex Ratio */
    sendCommand(SSD1306_HEIGHT-1);                                      /* 32 or 64 COM lines -1*/
    sendCommand(SSD1306_SET_DISPLAY_OFFSET);                            /* Set display offset */
    sendCommand(0x00);                                                  /* 0 offset */
    sendCommand(SSD1306_SET_START_LINE | 0x00);                         /* Set first line as the start line of the display */
    sendCommand(SSD1306_SET_CHARGE_PUMP);                               /* Charge pump */
    sendCommand(0x14);                                                  /* Enable charge dump during display on */  //pretty sure it needs to be this for 3.3v
    sendCommand(SSD1306_MEMORY_ADDRESS_MODE);                           /* Set memory addressing mode */
    sendCommand(SSD1306_SET_LCOL_START_ADDRESS);                        /* Horizontal addressing mode */
    sendCommand(SSD1306_SEGMENT_REMAP | 0x01);                          /* Set segment remap with column address 127 mapped to segment 0 */
    sendCommand(SSD1306_COM_SCAN_INVERSE);                              /* Set com output scan direction, scan from com63 to com 0 */
    sendCommand(SSD1306_SET_COM_PINS_CONFIG);                           /* Set com pins hardware configuration */
    sendCommand(0x12);//0x12);            //TODO investigate this 0x02 for 32 height      0x12 for 64  //0x02 make everything seem zoomed but still wrong                          /* Alternative com pin configuration, disable com left/right remap */
    sendCommand(SSD1306_SET_CONTRAST);                                  /* Set contrast control */
    sendCommand(0x8F);                      //was 0x80                              /* Set Contrast to 128 */
    sendCommand(SSD1306_SET_PRECHARGE_PERIOD);                          /* Set pre-charge period */
    sendCommand(0xF1);                                                  /* Phase 1 period of 15 DCLK, Phase 2 period of 1 DCLK */
    sendCommand(SSD1306_SET_VCOM_DESELECT_LVL);                         /* Set Vcomh deselect level */
    sendCommand(0x40);                                                  /* Vcomh deselect level */
    sendCommand(SSD1306_ENTIRE_DISPLAY_RESUME);                         /* Entire display ON, resume to RAM content display */
    sendCommand(SSD1306_NORMAL_DISPLAY);                                /* Set Display in Normal Mode, 1 = ON, 0 = OFF */
    //sendCommand(SSD1306_DEACTIVATE_SCROLL);//add deactivate scroll //doesnt seem to change anything
    sendCommand(SSD1306_DISPLAY_ON);                                    /* Display on in normal mode */
}

void sendCommand (unsigned char command) {
    data[0] = 0x00;
    data[1] = command;

    i2c_transmit(data, 2);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//For Direct writes
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


void setCursor (unsigned char x, unsigned char p) {
    //Note each Page p has two lines data written dictates line
    sendCommand(SSD1306_SET_LCOL_START_ADDRESS | (x & 0x0F));
    sendCommand(SSD1306_SET_HCOL_START_ADDRESS | (x >> 4));
    sendCommand(SSD1306_SET_PAGE_START_ADDRESS | p);
}

void drawPixel (unsigned char x, unsigned char y, unsigned char clear) {


    //Im not convinced this i right
    if ((x >= SSD1306_WIDTH) || (y >= SSD1306_HEIGHT)) return;
    setCursor(x, y >> 3);
    data[0] = SSD1306_DATA_MODE;
    if (clear)
        data[1] = (1 << (y & 7));
    else
        data[1] = ~(1 << (y & 7));
    i2c_transmit(data, 2);
}

void drawDot (unsigned char x, unsigned char y)
{

    //( 4 lines per page,  (line4,line3,line2,line1) so 11000011 should be line 4 and 1
    if((x < SSD1306_WIDTH) && (y < SSD1306_HEIGHT))
    {
        setCursor(x, (y-1)/4);
        data[0] = SSD1306_DATA_MODE;
        data[1] = 0x03<<(y%4);

        i2c_transmit(data, 2);
    }
}

void drawHLine(int x, int y, int lineLen)
{
    if((y >= 0) && (y < SSD1306_HEIGHT)) // Y coord in bounds?
    {
        if(x < 0)  // Clip left
        {
            lineLen += x;
            x  = 0;
        }

        if((x + lineLen) > SSD1306_WIDTH) // Clip right
        {
            lineLen = (SSD1306_WIDTH - x);
        }

        if(lineLen > 0)  // Proceed only if line is at least 1 pixel
        {
            int bufflen = lineLen+1;
            unsigned char line;

            dataBuffer = malloc(bufflen);
            dataBuffer[0] = SSD1306_DATA_MODE;

            line = 0x03<<(y%4);
            while(lineLen--)
            {
                *(dataBuffer + lineLen+1) = line;
            }

            setCursor(x, (y-1)/4);  //should be x then y page (4 lines per page)
            i2c_transmit(dataBuffer, bufflen);
            free(dataBuffer);
        }
    }
}

void fillDisplay(unsigned char color) {
  unsigned char page, x;

  dataBuffer = malloc(129);
  dataBuffer[0] = SSD1306_DATA_MODE;
  for (page = 0; page < 8; page++) {
      setCursor(0, page);
    for (x = 1; x < 129; x++) {
        dataBuffer[x] = color;
    }
    i2c_transmit(dataBuffer, 129);
  }
  free(dataBuffer);
}

void drawImage(unsigned char x, unsigned char y, unsigned char sx,
                       unsigned char sy, const unsigned char img[],
                       unsigned char invert) {
  unsigned int j, t;
  unsigned char i, p, p0, p1, n, n1, b;

  if (((x + sx) > SSD1306_WIDTH) || ((y + sy) > SSD1306_HEIGHT) ||
      (sx == 0) || (sy == 0)) return;

  // Total bytes of the image array
  if (sy % 8)
    t = (sy / 8 + 1) * sx;
  else
    t = (sy / 8) * sx;
  p0 = y / 8;                 // first page index
  p1 = (y + sy - 1) / 8;      // last page index
  n = y % 8;                  // offset form begin of page

  n1 = (y + sy) % 8;
  if (n1) n1 = 8 - n1;

  j = 0;                      // bytes counter [0..t], or [0..(t+sx)]
  dataBuffer = malloc(sx + 1);       // allocate memory for the buf
  dataBuffer[0] = SSD1306_DATA_MODE; // fist item "send data mode"
  for (p=p0; p<(p1+1); p++) {
      setCursor(x, p);
    for (i=x; i<(x+sx); i++) {
      if (p == p0) {
        b = (img[j] << n) & 0xFF;
      } else if ((p == p1) && (j >= t)) {
        b = (img[j - sx] >> n1) & 0xFF;
      } else {
        b = ((img[j - sx] >> (8 - n)) & 0xFF) | ((img[j] << n) & 0xFF);
      };
      if (invert)
          dataBuffer[i - x + 1] = b;
      else
          dataBuffer[i - x + 1] = ~b;
      j++;
    }
    i2c_transmit(dataBuffer, sx + 1); // send the buf to display
  }
  free(dataBuffer);
}

void draw6x8Str(unsigned char x, unsigned char p, const char str[],
                        unsigned char invert, unsigned char underline) {
  unsigned char i, j, b, buf[FONT6X8_WIDTH + 1];
  unsigned int c;

  i = 0;
  buf[0] = SSD1306_DATA_MODE; // fist item "send data mode"
  while (str[i] != '\0') {
    if (str[i] > 191)
      c = (str[i] - 64) * FONT6X8_WIDTH;
    else
      c = str[i] * FONT6X8_WIDTH;
    if (x > (SSD1306_WIDTH - FONT6X8_WIDTH - 1))
    {
      x = 0;
      p++;
    };
    if (p > 7) p = 0;
    setCursor(x, p);
    for (j = 0; j < FONT6X8_WIDTH; j++)
    {
      if (underline)
        b = font6x8[(unsigned int)(c + j)] | 0x80;
      else
        b = font6x8[(unsigned int)(c + j)];
      if (invert)
        buf[j + 1] = b;
      else
        buf[j + 1] = ~b;
    };
    i2c_transmit(buf, FONT6X8_WIDTH + 1); // send the buf to display
    x += FONT6X8_WIDTH;
    i++;
  };
}

void draw12x16Str(unsigned char x, unsigned char y, const char str[],
                          unsigned char invert) {
  unsigned char i;
  unsigned int c;

  i = 0;
  while (str[i] != '\0') {
    if (str[i] > 191)
      c = (str[i] - 64) * FONT12X16_WIDTH * 2;
    else
      c = str[i] * FONT12X16_WIDTH * 2;
    drawImage(x, y, 12, 16, (unsigned char *) &font12x16[c], invert);
    i++;
    x += 12;
  };
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//For Buffered writes
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*  WARNING do use this requires a heap of 1k (or a global array in place of malloc()) --CCS doest give us a heap that big by default
 * So this isnt going to work on on a MCU with small RAM.
 */
//GAVE UP ON THIS SINCE IM NOT SURE I NEED IT ANYWAY... and way it is written it probably wont work for 64pixel version

unsigned char * ssd1306_buff_begin(int displayHeight,int displayWidth)
{
    if(displayBuff_ptr) //if already defined
      return displaybuffer;

    Height = displayHeight;
    Width = displayWidth;
    displayBufLen = displayWidth * 8;  //I think whether its 32 or 64 high there is always 8pages

    displayBuff_ptr = (unsigned char *) malloc(displayBufLen+1); //add one for command
    displaybuffer = displayBuff_ptr+1;  //create second pointer for data leave space for command

    if(displayBuff_ptr)
    {
        ssd1306_clearBuff();
        return displaybuffer;
    }
    else
        return NULL;    //Returns NULL if malloc didnt work.
}

void ssd1306_buff_end(void)
{
    if(displayBuff_ptr)
    {
        free(displayBuff_ptr);
        displayBuff_ptr = NULL;
        displaybuffer = NULL;
    }
}

void ssd1306_clearBuff(void)
{
    unsigned char * ptr = displaybuffer;
    int i;
    for (i=0;i<displayBufLen;i++)
    {
        *ptr = 0x00;
        ptr++;
    }
}

void ssd1306_fillBuff(void)
{
    unsigned char * ptr = displaybuffer;
    int i;
    for (i=0;i<displayBufLen;i++)
    {
        *ptr = 0xFF;
        ptr++;
    }
}

void ssd1306_UpdateDisplay(void)
{
    *(displayBuff_ptr) = SSD1306_DATA_MODE;

    setCursor(0, 0);  //was (0,page)

    i2c_transmit(dataBuffer, displayBufLen+1);
}

void ssd1306_drawPixel(unsigned int x, unsigned int y, int color)
{
    if((x < Width) && (y < Height))
    {
        int page = (y-1)/4;
        unsigned char line = 0x03<<(y%4);
      switch(color)
      {
       case SSD1306_WHITE:   displaybuffer[x + page*Width] |=  line; break;
       case SSD1306_BLACK:   displaybuffer[x + page*Width] &= ~line; break;
       case SSD1306_INVERSE: displaybuffer[x + page*Width] ^=  line; break;
      }
    }
}

//drawFastHLineInternal(
//  int16_t x, int16_t y, int16_t w, uint16_t color) {
//
//  if((y >= 0) && (y < HEIGHT)) { // Y coord in bounds?
//    if(x < 0) { // Clip left
//      w += x;
//      x  = 0;
//    }
//    if((x + w) > WIDTH) { // Clip right
//      w = (WIDTH - x);
//    }
//    if(w > 0) { // Proceed only if width is positive
//      uint8_t *pBuf = &buffer[(y / 8) * WIDTH + x],
//               mask = 1 << (y & 7);
//      switch(color) {
//       case SSD1306_WHITE:               while(w--) { *pBuf++ |= mask; }; break;
//       case SSD1306_BLACK: mask = ~mask; while(w--) { *pBuf++ &= mask; }; break;
//       case SSD1306_INVERSE:             while(w--) { *pBuf++ ^= mask; }; break;
//      }
//    }
//  }
//}


