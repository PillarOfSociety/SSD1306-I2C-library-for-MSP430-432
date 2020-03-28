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

unsigned char data [2];
unsigned char *dataBuffer;      //used for direct writes
unsigned char * displayBuff_ptr = NULL;    //full buffer
unsigned char* displaybuffer = NULL;  //used for data portion of buffered writes
unsigned int displayBufLen;

int Height,Width;

//******************************************************************************************************************************************
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

//******************************************************************************************************************************************
void sendCommand (unsigned char command) {
    data[0] = 0x00;
    data[1] = command;

    i2c_transmit(data, 2);
}

//******************************************************************************************************************************************
void setCursor (unsigned char x, unsigned char p) {
    //Note each Page p has two lines data written dictates line
    sendCommand(SSD1306_SET_LCOL_START_ADDRESS | (x & 0x0F));
    sendCommand(SSD1306_SET_HCOL_START_ADDRESS | (x >> 4));
    sendCommand(SSD1306_SET_PAGE_START_ADDRESS | p);
}

//******************************************************************************************************************************************
void drawPixel (unsigned char x, unsigned char y, unsigned char clear) {

    if ((x >= SSD1306_WIDTH) || (y >= SSD1306_HEIGHT)) return;
    setCursor(x, y >> 3);
    data[0] = SSD1306_DATA_MODE;
    if (clear)
        data[1] = (1 << (y & 7));
    else
        data[1] = ~(1 << (y & 7));
    i2c_transmit(data, 2);
}

//******************************************************************************************************************************************
void drawDot (unsigned char x, unsigned char y) {

    if((x < SSD1306_WIDTH) && (y < SSD1306_HEIGHT))
    {
        setCursor(x, y/2);
        data[0] = SSD1306_DATA_MODE;

        if(y%2)
            data[1] = 0x02;
        else
            data[1] = 0x20;

        i2c_transmit(data, 2);
    }
}

//******************************************************************************************************************************************
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
//******************************************************************************************************************************************
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
//******************************************************************************************************************************************
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
//******************************************************************************************************************************************
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
//new stuff for buffered writes
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
unsigned char * ssd1306_buff_begin(int displayHeight,int displayWidth)
{
    if(displayBuff_ptr) //if already defined
      return displaybuffer;

    Height = displayHeight;
    Width = displayWidth;
    displayBufLen = displayWidth * ((displayHeight + 7) / 8);

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
void ssd1306_test(void) // int color, unsigned char y) //mostly just a test
{
    //try to use fill with just one line?
    unsigned char page, x;

    dataBuffer = malloc(135);
    dataBuffer[0] = SSD1306_DATA_MODE;

      setCursor(0, 1);  //was (0,page)
      for (x = 1; x < 129; x++)
      {
          dataBuffer[x] = 0xCF; //00110011  ( 4 lines per page,  (line4,line3,line2,line1) so 11000011 should be line 4 and 1
      }

      i2c_transmit(dataBuffer, 129);

      setCursor(0, 4);  //was (0,page)

      for (x = 1; x < 129; x++)
      {
          dataBuffer[x] = 0xC3; //00110011  ( 4 lines per page,  (line4,line3,line2,line1) so 11000011 should be line 4 and 1
      }
      i2c_transmit(dataBuffer, 129);

      setCursor(0, 6);  //was (0,page)
      for (x = 1; x < 135; x++)
      {
          dataBuffer[x] = 0xC3; //00110011  ( 4 lines per page,  (line4,line3,line2,line1) so 11000011 should be line 4 and 1
      }
      i2c_transmit(dataBuffer, 135);

    free(dataBuffer);
}



void ssd1306_display(void)
{
    //so some setup so we can dump the buffer to screen
    sendCommand(SSD1306_SET_PAGE_ADDRESS );  //0x22
    sendCommand(0);                        // Page start address
    sendCommand(0xFF);                  // Page end (not really, but works here)
    sendCommand(SSD1306_SET_COLUMN_ADDRESS);   //0x21  SSD1306_SET_COLUMN_ADDRESS
    sendCommand(0);             // Column start address
    sendCommand(Width - 1);     // Column end address

    displayBuff_ptr[0] = SSD1306_DATA_MODE;
    //write everything that is in buffer to display.
    i2c_transmit(displayBuff_ptr, displayBufLen+1);
}


void ssd1306_drawPixel(unsigned int x, unsigned int y, int color)
{
    if((x < Width) && (y < Height))
    {
      // Pixel is in-bounds. Rotate coordinates if needed.
        /* adafruits comments on what the rotations mean sucks so I dont know whats what.
      switch(getRotation())
      {
       case 1:
        ssd1306_swap(x, y);
        x = Width - x - 1;
        break;
       case 2:
       */
        x = Width  - x - 1;
        y = Height - y - 1;
        /*
        break;
       case 3:
        ssd1306_swap(x, y);
        y = HEIGHT - y - 1;
        break;
      }
      */
      switch(color)
      {
       case SSD1306_WHITE:   displaybuffer[x + (y/8)*Width] |=  (1 << (y&7)); break;
       case SSD1306_BLACK:   displaybuffer[x + (y/8)*Width] &= ~(1 << (y&7)); break;
       case SSD1306_INVERSE: displaybuffer[x + (y/8)*Width] ^=  (1 << (y&7)); break;
      }
    }
}

