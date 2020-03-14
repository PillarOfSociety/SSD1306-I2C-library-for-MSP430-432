/*
 * msp430_ssd1306_Example.c
 *
 *  Created on: Mar 13, 2020
 *      Author: AJS
 */
#include <msp430.h>
#include "msp430_ssd1306_Example.h"
#include "SSD1306_Driver/ssd1306_lib.h"

#if defined (__MSP430FR5994__)
void msp430Example(void)
{
    P1DIR |= (0x01| 0x02);      // Setup green and red led output direction (on demo board)

    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode

    ssd1306Init();

    for(;;) {
        volatile unsigned int i;            // volatile to prevent optimization

        P1OUT ^= 0x01;                      // Toggle P1.0 using exclusive-OR
        P1OUT ^=0x02;

        fillDisplay (FILL_Black);    //clears screen
        //draw6x8Str(0, 3, "Hello", 1, 0);  //currently too small on 32 height screen
        draw12x16Str(0, 3, "Hello", 1);
        draw12x16Str(0, 22, "World", 1);  //previous line font height +3 for spacing


        i = 1000;                          // SW Delay//should be 1 sec at 1mhz
        do
        {
            i--;
            __delay_cycles(1000);               // Delay between transmissions
        }
        while(i != 0);

        fillDisplay (FILL_White);

        i = 1000;                          // SW Delay//should be 1 sec at 1mhz
        do
        {
            i--;
            __delay_cycles(1000);               // Delay between transmissions
        }
        while(i != 0);

    }
}
#endif
