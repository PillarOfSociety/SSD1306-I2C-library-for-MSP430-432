/*
 * msp430_ssd1306_Example.c
 *
 *  Created on: Mar 13, 2020
 *      Author: AJS
 */
#include <msp430.h>
#include "address.h"
#include <SSD1306_Driver/ssd1306_Example.h>
#include "SSD1306_Driver/ssd1306_lib.h"

#if defined (__MSP430FR5994__)
void msp430Example(void)
{
    int i;
    const unsigned char ti_logo[] = {
    //   0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17    18    19    20    21    22    23    24    25    26    27
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0x80, 0x00, 0x00, 0x80, 0x98, 0x08, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00,
    //  28    29    30    31    32    33    34    35    36    37    38    39    40    41    42    43    44    45    46    47    48    49    50    51    52    53    54    55
      0x00, 0x10, 0x30, 0x70, 0xF0, 0xF0, 0xF0, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0x61, 0x01, 0x00, 0xF0, 0x3F, 0x01, 0x00, 0xE0, 0xE1, 0xF9, 0xFF, 0xFF, 0xFF, 0xF0, 0x00,
    //  56    57    58    59    60    61    62    63    64    65    66    67    68    69    70    71    72    73    74    75    76    77    78    79    80    81    82    83
      0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0x0F, 0x07, 0x03, 0x03, 0x03, 0x07, 0x1F, 0x60, 0xE0, 0xE0, 0xE1, 0xE0, 0xE0, 0x61, 0x31, 0x1F, 0x0F, 0x07, 0x07, 0x03, 0x03, 0x00,
    //  84    85    86    87    88    89    90    91    92    93    94    95    96    97    98    99   100   101   102   103   104   105   106   107   108   109   110   111
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    P1DIR |= (0x01| 0x02);      // Setup green and red led output direction (on demo board)

    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode

    ssd1306Init();

    for(;;) {

        P1OUT ^= 0x01;                      // Toggle P1.0 using exclusive-OR
        P1OUT ^=0x02;

        fillDisplay (FILL_Black);    //clears screen
        //draw6x8Str(0, 3, "Hello", 1, 0);  //currently too small on 32 height screen
        draw6x8Str(5, 0, "Hello", 1, 0);;
        draw12x16Str(20, 12, "World", 1);

        myDelay();

        fillDisplay (FILL_White);

        myDelay();
        fillDisplay (FILL_Black);    //clears screen

        myDelay();
        drawImage(100, 3, 28, 28, ti_logo, 1);

        myDelay();
        draw6x8Str(5, 0, "Count", 1,0);
        draw15x24Number(20, 8, 4,1);

        /*
        myDelay();
        fillDisplay (FILL_Black);
        //drawHLine(5, 3, 10);  //will be overwritten by "beginning" line below
        drawHLine(5, 6, 10);
        drawHLine(5, 12, 10);

        drawHLine(0, 0, 100); //beginning
        drawHLine(30, 31, 100); //end

        myDelay();
        fillDisplay (FILL_Black);

        //should make a square
        drawDot (120, 0);
        drawDot (120, 5);
        drawDot (125, 0);
        drawDot (125, 5);

        //myDelay();
        //fillDisplay (FILL_Black);
        //ssd1306_test();

        myDelay();
        fillDisplay (FILL_Black);    //clears screen

        //Try stuff with buffer
        if(ssd1306_buff_begin(SSD1306_SHORT_HEIGHT,SSD1306_WIDTH)==0) //check that memory could be allocated
        {
            draw12x16Str(30, 10, "ERROR", 1); //Need to make heap +1k in project settings
        }
        else
        {
            ssd1306_drawPixel(10, 10, SSD1306_WHITE);
            ssd1306_drawPixel(10, 20, SSD1306_WHITE);
            ssd1306_drawPixel(20, 20, SSD1306_WHITE);
            ssd1306_drawPixel(20, 10, SSD1306_WHITE);
            ssd1306_UpdateDisplay();     //this turns everything upside down and backwards preminent
            myDelay();

            ssd1306_clearBuff();
            ssd1306_UpdateDisplay(); //should clear display

            myDelay();

            ssd1306_fillBuff(); //should turn it all white
            ssd1306_UpdateDisplay();

            ssd1306_buff_end();
        }
*/
        myDelay();
    }
}

void myDelay(void)
{
    volatile unsigned int i;            // volatile to prevent optimization

    i = 1000;                          // SW Delay//should be 1 sec at 1mhz
    do
    {
        i--;
        __delay_cycles(1000);               // Delay between transmissions
    }
    while(i != 0);
}
#endif
